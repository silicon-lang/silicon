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


silicon::ast::BinaryOperation::BinaryOperation(binary_operation_t op, Node *left, Node *right) : op(op), left(left),
                                                                                                 right(right) {
}

silicon::ast::Node *
silicon::ast::BinaryOperation::create(compiler::Context *ctx, binary_operation_t op, Node *left, Node *right) {
    auto *node = new BinaryOperation(op, left, right);

    node->loc = parse_location(ctx->loc);

    return node;
}

llvm::Value *silicon::ast::BinaryOperation::codegen(compiler::Context *ctx) {
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

silicon::node_t silicon::ast::BinaryOperation::type() {
    return node_t::BINARY_OP;
}

silicon::value_pair_t *silicon::ast::BinaryOperation::parse_pair(compiler::Context *ctx) {
    auto *pair = new value_pair_t();

    llvm::Type *expected_type = ctx->expected_type;

    if (!expected_type) {
        bool isLeftNum = left->type(node_t::NUMBER_LIT);
        bool isRightNum = right->type(node_t::NUMBER_LIT);

        if (isLeftNum) {
            if (isRightNum) left->codegen(ctx);
            else {
                pair->right = right->codegen(ctx);

                ctx->expected_type = pair->right->getType();

                pair->left = left->codegen(ctx);

                ctx->expected_type = expected_type;
            }
        } else if (isRightNum) {
            pair->left = left->codegen(ctx);

            ctx->expected_type = pair->left->getType();

            pair->right = right->codegen(ctx);

            ctx->expected_type = expected_type;
        }
    }

    if (!pair->left) pair->left = left->codegen(ctx);
    if (!pair->right) pair->right = right->codegen(ctx);

    return pair;
}

llvm::Value *silicon::ast::BinaryOperation::assign(compiler::Context *ctx) {
    llvm::Type *expected_type = ctx->expected_type;

    llvm::Type *llvm_type = nullptr;
    std::string name;

    if (left->type(node_t::VARIABLE_DEFINITION)) {
        auto *l = (VariableDefinition *) left;

        name = l->getName();
        llvm_type = l->getLLVMType();

        if (!llvm_type) {
            llvm::Value *r = right->codegen(ctx);

            ctx->expected_type = r->getType();

            l->codegen(ctx);

            ctx->expected_type = expected_type;

            ctx->store(name, r);

            return r;
        }

        l->codegen(ctx);
    } else if (left->type(node_t::VARIABLE)) {
        auto *l = (Variable *) left;

        name = l->getName();
        llvm_type = ctx->get_alloca(l->getName())->getAllocatedType();
    } else fail_codegen("Expected left side of the equation to be a variable");

    ctx->expected_type = llvm_type;

    llvm::Value *r = right->codegen(ctx);

    ctx->expected_type = expected_type;

    ctx->store(name, r);

    return r;
}

llvm::Value *silicon::ast::BinaryOperation::multiply(compiler::Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("Expected both sides of operation to have the same type");

    llvm::Type *type = detect_type(l);

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateMul(l, r);

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFMul(l, r);

    fail_codegen("Unsupported operation");
}

llvm::Value *silicon::ast::BinaryOperation::divide(compiler::Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("Expected both sides of operation to have the same type");

    llvm::Type *type = detect_type(l);

    if (type->isIntegerTy()) {
        // TODO: unsigned: ctx->llvm_ir_builder.CreateUDiv(l, r)
        return ctx->llvm_ir_builder.CreateSDiv(l, r);
    }

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFDiv(l, r);

    fail_codegen("Unsupported operation");
}

llvm::Value *silicon::ast::BinaryOperation::remainder(compiler::Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("Expected both sides of operation to have the same type");

    llvm::Type *type = detect_type(l);

    if (type->isIntegerTy()) {
        // TODO: unsigned: ctx->llvm_ir_builder.CreateURem(l, r)
        return ctx->llvm_ir_builder.CreateSRem(l, r);
    }

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFRem(l, r);

    fail_codegen("Unsupported operation");
}

llvm::Value *silicon::ast::BinaryOperation::add(compiler::Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("Expected both sides of operation to have the same type");

    llvm::Type *type = detect_type(l);

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateAdd(l, r);

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFAdd(l, r);

    fail_codegen("Unsupported operation");
}

llvm::Value *silicon::ast::BinaryOperation::sub(compiler::Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("Expected both sides of operation to have the same type");

    llvm::Type *type = detect_type(l);

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateSub(l, r);

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFSub(l, r);

    fail_codegen("Unsupported operation");
}

llvm::Value *silicon::ast::BinaryOperation::bw_xor(compiler::Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("Expected both sides of operation to have the same type");

    llvm::Type *type = detect_type(l);

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateXor(l, r);

    fail_codegen("Unsupported operation");
}

llvm::Value *silicon::ast::BinaryOperation::bw_and(compiler::Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("Expected both sides of operation to have the same type");

    llvm::Type *type = detect_type(l);

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateAnd(l, r);

    fail_codegen("Unsupported operation");
}

llvm::Value *silicon::ast::BinaryOperation::bw_or(compiler::Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("Expected both sides of operation to have the same type");

    llvm::Type *type = detect_type(l);

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateOr(l, r);

    fail_codegen("Unsupported operation");
}

llvm::Value *silicon::ast::BinaryOperation::bw_left_shift(compiler::Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("Expected both sides of operation to have the same type");

    llvm::Type *type = detect_type(l);

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateShl(l, r);

    fail_codegen("Unsupported operation");
}

llvm::Value *silicon::ast::BinaryOperation::bw_right_shift(compiler::Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("Expected both sides of operation to have the same type");

    llvm::Type *type = detect_type(l);

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateAShr(l, r);

    fail_codegen("Unsupported operation");
}

llvm::Value *silicon::ast::BinaryOperation::bw_u_right_shift(compiler::Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("Expected both sides of operation to have the same type");

    llvm::Type *type = detect_type(l);

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateLShr(l, r);

    fail_codegen("Unsupported operation");
}

llvm::Value *silicon::ast::BinaryOperation::lt(compiler::Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("Expected both sides of operation to have the same type");

    llvm::Type *type = detect_type(l);

    if (type->isIntegerTy()) {
        // TODO: unsigned: ctx->llvm_ir_builder.CreateICmpULT(l, r)
        return ctx->llvm_ir_builder.CreateICmpSLT(l, r);
    }

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFCmpOLT(l, r);

    fail_codegen("Unsupported operation");
}

llvm::Value *silicon::ast::BinaryOperation::lte(compiler::Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("Expected both sides of operation to have the same type");

    llvm::Type *type = detect_type(l);

    if (type->isIntegerTy()) {
        // TODO: unsigned: ctx->llvm_ir_builder.CreateICmpULE(l, r)
        return ctx->llvm_ir_builder.CreateICmpSLE(l, r);
    }

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFCmpOLE(l, r);

    fail_codegen("Unsupported operation");
}

llvm::Value *silicon::ast::BinaryOperation::eq(compiler::Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("Expected both sides of operation to have the same type");

    llvm::Type *type = detect_type(l);

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateICmpEQ(l, r);

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFCmpOEQ(l, r);

    fail_codegen("Unsupported operation");
}

llvm::Value *silicon::ast::BinaryOperation::ne(compiler::Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("Expected both sides of operation to have the same type");

    llvm::Type *type = detect_type(l);

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateICmpNE(l, r);

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFCmpONE(l, r);

    fail_codegen("Unsupported operation");
}

llvm::Value *silicon::ast::BinaryOperation::gte(compiler::Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("Expected both sides of operation to have the same type");

    llvm::Type *type = detect_type(l);

    if (type->isIntegerTy()) {
        // TODO: unsigned: ctx->llvm_ir_builder.CreateICmpUGE(l, r)
        return ctx->llvm_ir_builder.CreateICmpSGE(l, r);
    }

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFCmpOGE(l, r);

    fail_codegen("Unsupported operation");
}

llvm::Value *silicon::ast::BinaryOperation::gt(compiler::Context *ctx) {
    value_pair_t *pair = parse_pair(ctx);

    llvm::Value *l = pair->left;
    llvm::Value *r = pair->right;

    if (!compare_types(l, r))
        fail_codegen("Expected both sides of operation to have the same type");

    llvm::Type *type = detect_type(l);

    if (type->isIntegerTy()) {
        // TODO: unsigned: ctx->llvm_ir_builder.CreateICmpUGT(l, r)
        return ctx->llvm_ir_builder.CreateICmpSGT(l, r);
    }

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFCmpOGT(l, r);

    fail_codegen("Unsupported operation");
}
