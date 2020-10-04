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


using namespace silicon;
using namespace ast;
using namespace compiler;


UnaryOperation::UnaryOperation(const string &location, unary_operation_t op, Node *node, bool suffix) : op(op), node(node), suffix(suffix) {
    this->location = location;
}

llvm::Value *UnaryOperation::codegen(Context *ctx) {
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

node_t UnaryOperation::type() {
    return node_t::UNARY_OP;
}

llvm::Value *UnaryOperation::increment(Context *ctx) {
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

    if (!operation) {
        if (suffix)
            fail_codegen(
                    "Error: Unsupported operation: <"
                    + parse_type(type)
                    + ">++"
            );
        else
            fail_codegen(
                    "Error: Unsupported operation: ++<"
                    + parse_type(type)
                    + ">"
            );
    }

    ctx->store(operation, var->get_pointer(ctx));

    if (suffix) return l;

    return operation;
}

llvm::Value *UnaryOperation::decrement(Context *ctx) {
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

    if (!operation) {
        if (suffix)
            fail_codegen(
                    "Error: Unsupported operation: <"
                    + parse_type(type)
                    + ">--"
            );
        else
            fail_codegen(
                    "Error: Unsupported operation: --<"
                    + parse_type(type)
                    + ">"
            );
    }

    ctx->store(operation, var->get_pointer(ctx));

    if (suffix) return l;

    return operation;
}

llvm::Value *UnaryOperation::negate(Context *ctx) {
    llvm::Value *n = node->codegen(ctx);

    llvm::Type *type = n->getType();

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateNSWNeg(n);

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFNeg(n);

    fail_codegen(
            "Error: Unsupported operation: -<"
            + parse_type(type)
            + ">"
    );
}

llvm::Value *UnaryOperation::not_op(Context *ctx) {
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

    fail_codegen(
            "Error: Unsupported operation: !<"
            + parse_type(type)
            + ">"
    );
}
