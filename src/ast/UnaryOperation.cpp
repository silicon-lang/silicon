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


#include "UnaryOperation.h"
#include "compiler/Context.h"


silicon::ast::UnaryOperation::UnaryOperation(unary_operation_t op, Node *node, bool suffix) : op(op), node(node),
                                                                                              suffix(suffix) {
}

silicon::ast::Node *
silicon::ast::UnaryOperation::create(compiler::Context *ctx, unary_operation_t op, Node *node, bool suffix) {
    auto *n = new UnaryOperation(op, node, suffix);

    n->loc = parse_location(ctx->loc);

    return n;
}

llvm::Value *silicon::ast::UnaryOperation::codegen(compiler::Context *ctx) {
    switch (op) {
        case unary_operation_t::PLUS_PLUS:
            return increment(ctx);
        case unary_operation_t::MINUS_MINUS:
            return decrement(ctx);
        case unary_operation_t::MINUS:
            return negate(ctx);
        case unary_operation_t::NOT:
            return not_op(ctx);
    }
}

silicon::node_t silicon::ast::UnaryOperation::type() {
    return silicon::node_t::UNARY_OP;
}

llvm::Value *silicon::ast::UnaryOperation::increment(compiler::Context *ctx) {
    if (!node->type(node_t::VARIABLE))
        fail_codegen("Expected variable");

    auto *var = (Variable *) node;

    llvm::Value *l = var->codegen(ctx);

    llvm::Type *expected_type = ctx->expected_type;

    ctx->expected_type = l->getType();

    llvm::Value *r = ctx->num_lit("1")->codegen(ctx);

    ctx->expected_type = expected_type;

    llvm::Type *type = l->getType();

    llvm::Value *operation = nullptr;

    if (type->isIntegerTy()) operation = ctx->llvm_ir_builder.CreateAdd(l, r);
    else if (type->isFloatingPointTy()) operation = ctx->llvm_ir_builder.CreateFAdd(l, r);

    if (!operation)
        fail_codegen("Unsupported operation");

    ctx->store(var->getName(), operation);

    if (suffix) return l;

    return operation;
}

llvm::Value *silicon::ast::UnaryOperation::decrement(compiler::Context *ctx) {
    if (!node->type(node_t::VARIABLE))
        fail_codegen("Expected variable");

    auto *var = (Variable *) node;

    llvm::Value *l = var->codegen(ctx);

    llvm::Type *expected_type = ctx->expected_type;

    ctx->expected_type = l->getType();

    llvm::Value *r = ctx->num_lit("1")->codegen(ctx);

    ctx->expected_type = expected_type;

    llvm::Type *type = l->getType();

    llvm::Value *operation = nullptr;

    if (type->isIntegerTy()) operation = ctx->llvm_ir_builder.CreateSub(l, r);
    else if (type->isFloatingPointTy()) operation = ctx->llvm_ir_builder.CreateFSub(l, r);

    if (!operation)
        fail_codegen("Unsupported operation");

    ctx->store(var->getName(), operation);

    if (suffix) return l;

    return operation;
}

llvm::Value *silicon::ast::UnaryOperation::negate(compiler::Context *ctx) {
    llvm::Value *n = node->codegen(ctx);

    llvm::Type *type = n->getType();

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateNSWNeg(n);

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFNeg(n);

    fail_codegen("Unsupported operation");
}

llvm::Value *silicon::ast::UnaryOperation::not_op(compiler::Context *ctx) {
    llvm::Value *value = node->codegen(ctx);

    llvm::Type *type = value->getType();

    if (type->isVoidTy()) return ctx->bool_lit(true)->codegen(ctx);

    if (type->isArrayTy()) return ctx->bool_lit(false)->codegen(ctx);

    if (type->isIntegerTy(1)) return ctx->llvm_ir_builder.CreateNot(value);

    llvm::Type *expected_type = ctx->expected_type;

    ctx->expected_type = type;

    llvm::Value *v = ctx->num_lit("0")->codegen(ctx);

    ctx->expected_type = expected_type;

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateICmpEQ(value, v);

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFCmpOEQ(value, v);

    fail_codegen("Unsupported operation");
}
