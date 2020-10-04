//
//   Copyright 2020 Ardalan Amini
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//


#include "BinaryOperation.h"
#include "compiler/Context.h"


using namespace std;
using namespace silicon;
using namespace ast;
using namespace compiler;


BinaryOperation::BinaryOperation(const string &location, binary_operation_t op, Node *left, Node *right) : op(op),
                                                                                                           left(left),
                                                                                                           right(right) {
    this->location = location;
}

llvm::Value *BinaryOperation::codegen(Context *ctx) {
    switch (op) {
        case binary_operation_t::ASSIGN:
            return assign(ctx);
        case binary_operation_t::STAR:
            return multiply(ctx);
        case binary_operation_t::SLASH:
            return divide(ctx);
        case binary_operation_t::PERCENT:
            return remainder(ctx);
        case binary_operation_t::PLUS:
            return add(ctx);
        case binary_operation_t::MINUS:
            return sub(ctx);
        case binary_operation_t::CARET:
            return bw_xor(ctx);
        case binary_operation_t::AND:
            return bw_and(ctx);
        case binary_operation_t::OR:
            return bw_or(ctx);
        case binary_operation_t::DOUBLE_LESSER:
            return bw_left_shift(ctx);
        case binary_operation_t::DOUBLE_BIGGER:
            return bw_right_shift(ctx);
        case binary_operation_t::TRIPLE_BIGGER:
            return bw_u_right_shift(ctx);
        case binary_operation_t::LESSER:
            return lt(ctx);
        case binary_operation_t::LESSER_EQUAL:
            return lte(ctx);
        case binary_operation_t::EQUAL:
            return eq(ctx);
        case binary_operation_t::NOT_EQUAL:
            return ne(ctx);
        case binary_operation_t::BIGGER_EQUAL:
            return gte(ctx);
        case binary_operation_t::BIGGER:
            return gt(ctx);
    }
}

node_t BinaryOperation::type() {
    return node_t::BINARY_OP;
}

value_pair_t *BinaryOperation::parse_pair(Context *ctx) {
    auto *pair = new value_pair_t();

    llvm::Type *expected_type = ctx->expected_type;

    bool isLeftDynamic = left->type(node_t::NUMBER_LIT) || left->type(node_t::NULL_PTR);
    bool isRightDynamic = right->type(node_t::NUMBER_LIT) || right->type(node_t::NULL_PTR);

    if (isLeftDynamic) {
        if (isRightDynamic) left->codegen(ctx);
        else {
            pair->right = right->codegen(ctx);

            ctx->expected_type = pair->right->getType();

            pair->left = left->codegen(ctx);

            ctx->expected_type = expected_type;
        }
    } else if (isRightDynamic) {
        pair->left = left->codegen(ctx);

        ctx->expected_type = pair->left->getType();

        pair->right = right->codegen(ctx);

        ctx->expected_type = expected_type;
    }

    if (!pair->left) pair->left = left->codegen(ctx);
    if (!pair->right) pair->right = right->codegen(ctx);

    return pair;
}

llvm::Value *BinaryOperation::assign(Context *ctx) {
    llvm::Type *expected_type = ctx->expected_type;

    llvm::Type *llvm_type;
    string name;

    if (left->type(node_t::VARIABLE_DEFINITION)) {
        auto *l = (VariableDefinition *) left;

        name = l->getName();
        llvm_type = l->getLLVMType(ctx);

        if (!llvm_type) {
            llvm::Value *r = right->codegen(ctx);

            ctx->expected_type = r->getType();

            auto *alloca = l->codegen(ctx);

            ctx->expected_type = expected_type;

            ctx->store(r, alloca);

            return r;
        }

        auto *alloca = l->codegen(ctx);

        ctx->expected_type = llvm_type;

        llvm::Value *r = right->codegen(ctx);

        if (!compare_types(r->getType(), ctx->expected_type)) {
            fail_codegen(
                    "TypeError: Expected the right side of the operation to be <"
                    + parse_type(llvm_type)
                    + ">, got <"
                    + parse_type(r->getType())
                    + "> instead."
            );
        }

        ctx->expected_type = expected_type;

        ctx->store(r, alloca);

        return r;
    }

    if (left->type(node_t::VARIABLE)) {
        auto *l = (Variable *) left;

        name = l->getName();
        llvm_type = l->getType(ctx);

        ctx->expected_type = llvm_type;

        llvm::Value *r = right->codegen(ctx);

        if (llvm_type && !compare_types(r->getType(), ctx->expected_type)) {
            fail_codegen(
                    "TypeError: Expected the right side of the operation to be <"
                    + parse_type(llvm_type)
                    + ">, got <"
                    + parse_type(r->getType())
                    + "> instead."
            );
        }

        ctx->expected_type = expected_type;

        ctx->store(r, l->get_pointer(ctx));

        return r;
    }

    fail_codegen("Expected left side of the equation to be a variable");
}

llvm::Value *BinaryOperation::multiply(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = l->getType();

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateMul(l, r);

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFMul(l, r);

    fail_codegen(
            "Error: Unsupported operation: <"
            + parse_type(type)
            + "> * <"
            + parse_type(r->getType())
            + ">"
    );
}

llvm::Value *BinaryOperation::divide(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = l->getType();

    if (type->isIntegerTy()) {
        // TODO: unsigned: ctx->llvm_ir_builder.CreateUDiv(l, r)
        return ctx->llvm_ir_builder.CreateSDiv(l, r);
    }

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFDiv(l, r);

    fail_codegen(
            "Error: Unsupported operation: <"
            + parse_type(type)
            + "> / <"
            + parse_type(r->getType())
            + ">"
    );
}

llvm::Value *BinaryOperation::remainder(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = l->getType();

    if (type->isIntegerTy()) {
        // TODO: unsigned: ctx->llvm_ir_builder.CreateURem(l, r)
        return ctx->llvm_ir_builder.CreateSRem(l, r);
    }

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFRem(l, r);

    fail_codegen(
            "Error: Unsupported operation: <"
            + parse_type(type)
            + "> % <"
            + parse_type(r->getType())
            + ">"
    );
}

llvm::Value *BinaryOperation::add(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = l->getType();

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateAdd(l, r);

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFAdd(l, r);

    fail_codegen(
            "Error: Unsupported operation: <"
            + parse_type(type)
            + "> + <"
            + parse_type(r->getType())
            + ">"
    );
}

llvm::Value *BinaryOperation::sub(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = l->getType();

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateSub(l, r);

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFSub(l, r);

    fail_codegen(
            "Error: Unsupported operation: <"
            + parse_type(type)
            + "> - <"
            + parse_type(r->getType())
            + ">"
    );
}

llvm::Value *BinaryOperation::bw_xor(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = l->getType();

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateXor(l, r);

    fail_codegen(
            "Error: Unsupported operation: <"
            + parse_type(type)
            + "> ^ <"
            + parse_type(r->getType())
            + ">"
    );
}

llvm::Value *BinaryOperation::bw_and(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = l->getType();

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateAnd(l, r);

    fail_codegen(
            "Error: Unsupported operation: <"
            + parse_type(type)
            + "> & <"
            + parse_type(r->getType())
            + ">"
    );
}

llvm::Value *BinaryOperation::bw_or(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = l->getType();

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateOr(l, r);

    fail_codegen(
            "Error: Unsupported operation: <"
            + parse_type(type)
            + "> | <"
            + parse_type(r->getType())
            + ">"
    );
}

llvm::Value *BinaryOperation::bw_left_shift(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = l->getType();

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateShl(l, r);

    fail_codegen(
            "Error: Unsupported operation: <"
            + parse_type(type)
            + "> << <"
            + parse_type(r->getType())
            + ">"
    );
}

llvm::Value *BinaryOperation::bw_right_shift(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = l->getType();

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateAShr(l, r);

    fail_codegen(
            "Error: Unsupported operation: <"
            + parse_type(type)
            + "> >> <"
            + parse_type(r->getType())
            + ">"
    );
}

llvm::Value *BinaryOperation::bw_u_right_shift(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = l->getType();

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateLShr(l, r);

    fail_codegen(
            "Error: Unsupported operation: <"
            + parse_type(type)
            + "> >>> <"
            + parse_type(r->getType())
            + ">"
    );
}

llvm::Value *BinaryOperation::lt(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = l->getType();

    if (type->isIntegerTy()) {
        // TODO: unsigned: ctx->llvm_ir_builder.CreateICmpULT(l, r)
        return ctx->llvm_ir_builder.CreateICmpSLT(l, r);
    }

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFCmpOLT(l, r);

    fail_codegen(
            "Error: Unsupported operation: <"
            + parse_type(type)
            + "> < <"
            + parse_type(r->getType())
            + ">"
    );
}

llvm::Value *BinaryOperation::lte(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = l->getType();

    if (type->isIntegerTy()) {
        // TODO: unsigned: ctx->llvm_ir_builder.CreateICmpULE(l, r)
        return ctx->llvm_ir_builder.CreateICmpSLE(l, r);
    }

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFCmpOLE(l, r);

    fail_codegen(
            "Error: Unsupported operation: <"
            + parse_type(type)
            + "> <= <"
            + parse_type(r->getType())
            + ">"
    );
}

llvm::Value *BinaryOperation::eq(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = l->getType();

    if (type->isVoidTy()) return ctx->bool_lit(true)->codegen(ctx);

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateICmpEQ(l, r);

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFCmpOEQ(l, r);

    fail_codegen(
            "Error: Unsupported operation: <"
            + parse_type(type)
            + "> == <"
            + parse_type(r->getType())
            + ">"
    );
}

llvm::Value *BinaryOperation::ne(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = l->getType();

    if (type->isVoidTy()) return ctx->bool_lit(false)->codegen(ctx);

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateICmpNE(l, r);

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFCmpONE(l, r);

    fail_codegen(
            "Error: Unsupported operation: <"
            + parse_type(type)
            + "> != <"
            + parse_type(r->getType())
            + ">"
    );
}

llvm::Value *BinaryOperation::gte(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = l->getType();

    if (type->isIntegerTy()) {
        // TODO: unsigned: ctx->llvm_ir_builder.CreateICmpUGE(l, r)
        return ctx->llvm_ir_builder.CreateICmpSGE(l, r);
    }

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFCmpOGE(l, r);

    fail_codegen(
            "Error: Unsupported operation: <"
            + parse_type(type)
            + "> >= <"
            + parse_type(r->getType())
            + ">"
    );
}

llvm::Value *BinaryOperation::gt(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = l->getType();

    if (type->isIntegerTy()) {
        // TODO: unsigned: ctx->llvm_ir_builder.CreateICmpUGT(l, r)
        return ctx->llvm_ir_builder.CreateICmpSGT(l, r);
    }

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFCmpOGT(l, r);

    fail_codegen(
            "Error: Unsupported operation: <"
            + parse_type(type)
            + "> > <"
            + parse_type(r->getType())
            + ">"
    );
}
