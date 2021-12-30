//
//   Copyright 2021 Ardalan Amini
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


#include "silicon/CodeGen/CGBinaryOperation.h"
#include "silicon/CodeGen/CGType.h"
#include "silicon/CodeGen/CGVariable.h"
#include "silicon/CodeGen/CGVariableDefinition.h"


using namespace std;
using namespace llvm;
using namespace silicon::codegen;
using namespace silicon::parser::AST;


CGBinaryOperation::CGBinaryOperation(BinaryOperation *node) : Node{node}, BinaryOperation{node} {
}

string CGBinaryOperation::stringify_operator() {
    switch (op) {
        case binary_operation_t::ASSIGN:
            return "=";
        case binary_operation_t::STAR:
            return "*";
        case binary_operation_t::SLASH:
            return "/";
        case binary_operation_t::PERCENT:
            return "%";
        case binary_operation_t::PLUS:
            return "+";
        case binary_operation_t::MINUS:
            return "-";
        case binary_operation_t::CARET:
            return "^";
        case binary_operation_t::AND:
            return "&";
        case binary_operation_t::OR:
            return "|";
        case binary_operation_t::DOUBLE_LESSER:
            return "<<";
        case binary_operation_t::DOUBLE_BIGGER:
            return ">>";
        case binary_operation_t::TRIPLE_BIGGER:
            return ">>>";
        case binary_operation_t::LESSER:
            return "<";
        case binary_operation_t::LESSER_EQUAL:
            return "<=";
        case binary_operation_t::EQUAL:
            return "==";
        case binary_operation_t::NOT_EQUAL:
            return "!=";
        case binary_operation_t::BIGGER_EQUAL:
            return ">=";
        case binary_operation_t::BIGGER:
            return ">";
        case binary_operation_t::STAR_STAR:
            return "**";
        case binary_operation_t::AND_AND:
            return "&&";
        case binary_operation_t::OR_OR:
            return "||";
        case binary_operation_t::CAST:
            return "as";
    }
}

Value *CGBinaryOperation::codegen(Context *ctx) {
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
        case binary_operation_t::STAR_STAR:
        case binary_operation_t::AND_AND:
        case binary_operation_t::OR_OR:
        case binary_operation_t::CAST:
        default:
            fail("Unsupported binary operation!");
    }
}

void CGBinaryOperation::unsupported_op(Context *ctx, llvm::Type *t1, llvm::Type *t2) noexcept {
    fail(
            "Error: Unsupported operation: <" + ctx->stringify_type(t1) + "> "
            + stringify_operator()
            + " <" + ctx->stringify_type(t2) + ">"
    );
}

value_pair_t *CGBinaryOperation::parse_pair(Context *ctx) {
    auto *pair = new value_pair_t();

    auto *l = dynamic_cast<CGNode *>(left);
    auto *r = dynamic_cast<CGNode *>(right);

    llvm::Type *expected_type = ctx->expected_type;

    bool isLeftDynamic = l->is_node(node_t::NUMBER_LIT) || l->is_node(node_t::NULL_PTR);
    bool isRightDynamic = r->is_node(node_t::NUMBER_LIT) || r->is_node(node_t::NULL_PTR);

    if (isLeftDynamic) {
        if (isRightDynamic) l->codegen(ctx);
        else {
            pair->right = r->codegen(ctx);

            ctx->expected_type = pair->right->getType();

            pair->left = l->codegen(ctx);

            ctx->expected_type = expected_type;
        }
    } else if (isRightDynamic) {
        pair->left = l->codegen(ctx);

        ctx->expected_type = pair->left->getType();

        pair->right = r->codegen(ctx);

        ctx->expected_type = expected_type;
    }

    if (!pair->left) pair->left = l->codegen(ctx);

    if (!pair->right) pair->right = r->codegen(ctx);

    return pair;
}

Value *CGBinaryOperation::assign(Context *ctx) {
    auto *l = dynamic_cast<CGNode *>(left);
    auto *r = dynamic_cast<CGNode *>(right);

    llvm::Type *expected_type = ctx->expected_type;

    llvm::Type *llvm_type;

    if (l->is_node(node_t::VARIABLE_DEFINITION)) {
        auto *lVD = dynamic_cast<CGVariableDefinition *>(l);

        llvm_type = lVD->get_type(ctx);

        if (!llvm_type) {
            Value *rV = r->codegen(ctx);

            ctx->expected_type = rV->getType();

            auto *alloca = (AllocaInst *)lVD->codegen(ctx);

            ctx->expected_type = expected_type;

            StoreInst *store = ctx->store(rV, alloca);

            unsigned alignment = alloca->getAlignment();

            if (alignment > 0) store->setAlignment(alignment);

            return rV;
        }

        auto *alloca = (AllocaInst *)lVD->codegen(ctx);

        ctx->expected_type = llvm_type;

        Value *rV = r->codegen(ctx);

        if (!ctx->compare_types(rV->getType(), ctx->expected_type)) {
            fail(
                    "TypeError: Expected the right side of the operation to be <"
                    + ctx->stringify_type(llvm_type)
                    + ">, got <"
                    + ctx->stringify_type(rV->getType())
                    + "> instead."
            );
        }

        ctx->expected_type = expected_type;

        StoreInst *store = ctx->store(rV, alloca);

        unsigned alignment = alloca->getAlignment();

        if (alignment > 0) store->setAlignment(alignment);

        return rV;
    }

    if (l->is_node(node_t::VARIABLE)) {
        auto *lV = dynamic_cast<CGVariable *>(l);

        llvm_type = lV->get_type(ctx);

        ctx->expected_type = llvm_type;

        Value *rV = r->codegen(ctx);

        if (llvm_type && !ctx->compare_types(rV->getType(), ctx->expected_type)) {
            fail(
                    "TypeError: Expected the right side of the operation to be <"
                    + ctx->stringify_type(llvm_type)
                    + ">, got <"
                    + ctx->stringify_type(rV->getType())
                    + "> instead."
            );
        }

        ctx->expected_type = expected_type;

        ctx->store(rV, lV->get_pointer(ctx));

        return rV;
    }

    fail("Expected left side of the equation to be a variable");
}

Value *CGBinaryOperation::multiply(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    Value *left = pair->left;
    Value *right = pair->right;

    if (!ctx->compare_types(left, right))
        fail("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = left->getType();

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateNSWMul(left, right);

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFMul(left, right);

    unsupported_op(ctx, type, right->getType());
}

Value *CGBinaryOperation::divide(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    Value *left = pair->left;
    Value *right = pair->right;

    if (!ctx->compare_types(left, right))
        fail("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = left->getType();

    if (type->isIntegerTy()) {
        // TODO: unsigned: ctx->llvm_ir_builder.CreateUDiv(left, right)
        return ctx->llvm_ir_builder.CreateSDiv(left, right);
    }

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFDiv(left, right);

    unsupported_op(ctx, type, right->getType());
}

Value *CGBinaryOperation::remainder(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    Value *left = pair->left;
    Value *right = pair->right;

    if (!ctx->compare_types(left, right))
        fail("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = left->getType();

    if (type->isIntegerTy()) {
        // TODO: unsigned: ctx->llvm_ir_builder.CreateURem(left, right)
        return ctx->llvm_ir_builder.CreateSRem(left, right);
    }

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFRem(left, right);

    unsupported_op(ctx, type, right->getType());
}

Value *CGBinaryOperation::add(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    Value *left = pair->left;
    Value *right = pair->right;

    if (!ctx->compare_types(left, right))
        fail("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = left->getType();

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateNSWAdd(left, right);

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFAdd(left, right);

    unsupported_op(ctx, type, right->getType());
}

Value *CGBinaryOperation::sub(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    Value *left = pair->left;
    Value *right = pair->right;

    if (!ctx->compare_types(left, right))
        fail("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = left->getType();

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateNSWSub(left, right);

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFSub(left, right);

    unsupported_op(ctx, type, right->getType());
}

Value *CGBinaryOperation::bw_xor(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    Value *left = pair->left;
    Value *right = pair->right;

    if (!ctx->compare_types(left, right))
        fail("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = left->getType();

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateXor(left, right);

    unsupported_op(ctx, type, right->getType());
}

Value *CGBinaryOperation::bw_and(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    Value *left = pair->left;
    Value *right = pair->right;

    if (!ctx->compare_types(left, right))
        fail("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = left->getType();

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateAnd(left, right);

    unsupported_op(ctx, type, right->getType());
}

Value *CGBinaryOperation::bw_or(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    Value *left = pair->left;
    Value *right = pair->right;

    if (!ctx->compare_types(left, right))
        fail("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = left->getType();

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateOr(left, right);

    unsupported_op(ctx, type, right->getType());
}

Value *CGBinaryOperation::bw_left_shift(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    Value *left = pair->left;
    Value *right = pair->right;

    if (!ctx->compare_types(left, right))
        fail("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = left->getType();

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateShl(left, right);

    unsupported_op(ctx, type, right->getType());
}

Value *CGBinaryOperation::bw_right_shift(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    Value *left = pair->left;
    Value *right = pair->right;

    if (!ctx->compare_types(left, right))
        fail("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = left->getType();

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateAShr(left, right);

    unsupported_op(ctx, type, right->getType());
}

Value *CGBinaryOperation::bw_u_right_shift(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    Value *left = pair->left;
    Value *right = pair->right;

    if (!ctx->compare_types(left, right))
        fail("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = left->getType();

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateLShr(left, right);

    unsupported_op(ctx, type, right->getType());
}

Value *CGBinaryOperation::lt(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    Value *left = pair->left;
    Value *right = pair->right;

    if (!ctx->compare_types(left, right))
        fail("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = left->getType();

    if (type->isIntegerTy()) {
        // TODO: unsigned: ctx->llvm_ir_builder.CreateICmpULT(l, r)
        return ctx->llvm_ir_builder.CreateICmpSLT(left, right);
    }

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFCmpOLT(left, right);

    unsupported_op(ctx, type, right->getType());
}

Value *CGBinaryOperation::lte(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    Value *left = pair->left;
    Value *right = pair->right;

    if (!ctx->compare_types(left, right))
        fail("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = left->getType();

    if (type->isIntegerTy()) {
        // TODO: unsigned: ctx->llvm_ir_builder.CreateICmpULE(left, right)
        return ctx->llvm_ir_builder.CreateICmpSLE(left, right);
    }

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFCmpOLE(left, right);

    unsupported_op(ctx, type, right->getType());
}

Value *CGBinaryOperation::eq(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    Value *left = pair->left;
    Value *right = pair->right;

    if (!ctx->compare_types(left, right))
        fail("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = left->getType();

//    if (type->isVoidTy()) return ctx->bool_lit(true)->codegen(ctx);

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateICmpEQ(left, right);

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFCmpOEQ(left, right);

    unsupported_op(ctx, type, right->getType());
}

Value *CGBinaryOperation::ne(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    Value *left = pair->left;
    Value *right = pair->right;

    if (!ctx->compare_types(left, right))
        fail("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = left->getType();

//    if (type->isVoidTy()) return ctx->bool_lit(false)->codegen(ctx);

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateICmpNE(left, right);

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFCmpONE(left, right);

    unsupported_op(ctx, type, right->getType());
}

Value *CGBinaryOperation::gte(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    Value *left = pair->left;
    Value *right = pair->right;

    if (!ctx->compare_types(left, right))
        fail("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = left->getType();

    if (type->isIntegerTy()) {
        // TODO: unsigned: ctx->llvm_ir_builder.CreateICmpUGE(left, right)
        return ctx->llvm_ir_builder.CreateICmpSGE(left, right);
    }

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFCmpOGE(left, right);

    unsupported_op(ctx, type, right->getType());
}

Value *CGBinaryOperation::gt(Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    Value *left = pair->left;
    Value *right = pair->right;

    if (!ctx->compare_types(left, right))
        fail("TypeError: Expected both sides of the operation to have the same type.");

    llvm::Type *type = left->getType();

    if (type->isIntegerTy()) {
        // TODO: unsigned: ctx->llvm_ir_builder.CreateICmpUGT(left, right)
        return ctx->llvm_ir_builder.CreateICmpSGT(left, right);
    }

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFCmpOGT(left, right);

    unsupported_op(ctx, type, right->getType());
}

Value *CGBinaryOperation::cast(Context *ctx) {
    // TODO: use ctx->cast_type()
    auto *r = dynamic_cast<CGNode *>(right);

    if (!r->is_node(node_t::TYPE)) fail("Error: Expected cast operation's right hand value to be a type");

    auto *l = dynamic_cast<CGNode *>(left);

    llvm::Value *v;
    llvm::Type *llvm_t = dynamic_cast<CGType *>(right)->typegen(ctx);

    if (l->is_node(node_t::NUMBER_LIT)
        && (llvm_t->isIntegerTy()
            || llvm_t->isFloatingPointTy())) {
        llvm::Type *expected_type = ctx->expected_type;

        ctx->expected_type = llvm_t;

        v = l->codegen(ctx);

        ctx->expected_type = expected_type;

        return v;
    }

    v = l->codegen(ctx);

    llvm::Type *t = v->getType();

    if (ctx->compare_types(t, llvm_t)) return v;

    if (llvm_t->isIntegerTy(1)) {
        if (t->isVoidTy()) return ctx->bool_lit(false);

        if (t->isArrayTy()) return ctx->bool_lit(true);
    }

    if (!llvm::CastInst::isCastable(t, llvm_t)) {
        fail(
                "Error: Unsupported cast: <"
                + ctx->stringify_type(t)
                + "> as <"
                + ctx->stringify_type(llvm_t)
                + ">"
        );
    }

    return ctx->llvm_ir_builder.CreateCast(
            llvm::CastInst::getCastOpcode(
                    v,
                    true,
                    llvm_t,
                    true
            ),
            v,
            llvm_t,
            "cast"
    );
}
