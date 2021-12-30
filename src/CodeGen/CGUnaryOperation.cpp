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


#include "silicon/CodeGen/CGUnaryOperation.h"
#include "silicon/CodeGen/CGVariable.h"


using namespace std;
using namespace llvm;
using namespace silicon::codegen;
using namespace silicon::parser::AST;


CGUnaryOperation::CGUnaryOperation(UnaryOperation *node) : Node{node}, UnaryOperation{node} {
}

Value *CGUnaryOperation::codegen(Context *ctx) {
    switch (op) {
        case unary_operation_t::PLUS_PLUS:
            return increment(ctx);
        case unary_operation_t::MINUS_MINUS:
            return decrement(ctx);
        case unary_operation_t::MINUS:
            return negate(ctx);
        case unary_operation_t::NOT:
            return not_op(ctx);
        case unary_operation_t::AND:
        case unary_operation_t::PLUS:
        case unary_operation_t::TILDE:
        default:
            fail("Unsupported unary operation!");
    }
}

string CGUnaryOperation::stringify_operator() {
    switch (op) {
        case unary_operation_t::AND:
            return "&";
        case unary_operation_t::PLUS:
            return "+";
        case unary_operation_t::PLUS_PLUS:
            return "++";
        case unary_operation_t::MINUS:
            return "-";
        case unary_operation_t::MINUS_MINUS:
            return "--";
        case unary_operation_t::NOT:
            return "!";
        case unary_operation_t::TILDE:
            return "~";
    }
}

void CGUnaryOperation::unsupported_op(Context *ctx, Type *type) noexcept {
    string opS = stringify_operator();

    if (suffix)
        fail(
                "Error: Unsupported operation: <"
                + ctx->stringify_type(type)
                + ">"
                + opS
        );

    fail(
            "Error: Unsupported operation: "
            + opS
            + "<"
            + ctx->stringify_type(type)
            + ">"
    );
}

Value *CGUnaryOperation::increment(Context *ctx) {
    if (!node->is_node(node_t::VARIABLE))
        fail("TypeError: Expected variable");

    auto *var = dynamic_cast<CGVariable *>(node);

    Value *l = var->codegen(ctx);

    Type *expected_type = ctx->expected_type;

    ctx->expected_type = l->getType();

    Value *r = ctx->number_lit("1");

    ctx->expected_type = expected_type;

    Type *type = l->getType();

    Value *operation = nullptr;

    if (type->isIntegerTy()) operation = ctx->llvm_ir_builder.CreateNSWAdd(l, r);
    else if (type->isFloatingPointTy()) operation = ctx->llvm_ir_builder.CreateFAdd(l, r);

    if (!operation) unsupported_op(ctx, type);

    ctx->store(operation, var->get_pointer(ctx));

    if (suffix) return l;

    return operation;
}

Value *CGUnaryOperation::decrement(Context *ctx) {
    if (!node->is_node(node_t::VARIABLE))
        fail("TypeError: Expected variable");

    auto *var = dynamic_cast<CGVariable *>(node);

    Value *l = var->codegen(ctx);

    Type *expected_type = ctx->expected_type;

    ctx->expected_type = l->getType();

    Value *r = ctx->number_lit("1");

    ctx->expected_type = expected_type;

    Type *type = l->getType();

    Value *operation = nullptr;

    if (type->isIntegerTy()) operation = ctx->llvm_ir_builder.CreateNSWSub(l, r);
    else if (type->isFloatingPointTy()) operation = ctx->llvm_ir_builder.CreateFSub(l, r);

    if (!operation) unsupported_op(ctx, type);

    ctx->store(operation, var->get_pointer(ctx));

    if (suffix) return l;

    return operation;
}

Value *CGUnaryOperation::negate(Context *ctx) {
    llvm::Value *n = dynamic_cast<CGNode *>(node)->codegen(ctx);

    llvm::Type *type = n->getType();

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateNSWNeg(n);

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFNeg(n);

    unsupported_op(ctx, type);
}

Value *CGUnaryOperation::not_op(Context *ctx) {
    llvm::Value *value = dynamic_cast<CGNode *>(node)->codegen(ctx);

    llvm::Type *type = value->getType();

    if (type->isVoidTy()) return ctx->bool_lit(true);

    if (type->isArrayTy()) return ctx->bool_lit(false);

    if (type->isIntegerTy(1)) return ctx->llvm_ir_builder.CreateNot(value);

    llvm::Type *expected_type = ctx->expected_type;

    ctx->expected_type = type;

    llvm::Value *v = ctx->number_lit("0");

    ctx->expected_type = expected_type;

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateICmpEQ(value, v);

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFCmpOEQ(value, v);

    unsupported_op(ctx, type);
}
