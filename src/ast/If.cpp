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


#include <utility>
#include "If.h"
#include "compiler/Context.h"


silicon::ast::If::If(Node *condition, std::vector<Node *> then_statements, std::vector<Node *> else_statements)
        : condition(condition),
          then_statements(std::move(then_statements)),
          else_statements(std::move(else_statements)) {
}

silicon::ast::If *
silicon::ast::If::create(compiler::Context *ctx, Node *condition, std::vector<Node *> then_statements,
                         std::vector<Node *> else_statements) {
    if (then_statements.empty()) then_statements = {ctx->null()};

    if (else_statements.empty()) else_statements = {ctx->null()};

    auto *node = new If(condition, std::move(then_statements), std::move(else_statements));

    node->loc = parse_location(ctx->loc);

    return node;
}

llvm::Value *silicon::ast::If::codegen(compiler::Context *ctx) {
    llvm::Value *conditionV = conditionCodegen(ctx);

    if (!conditionV)
        return nullptr;

    llvm::Function *function = ctx->llvm_ir_builder.GetInsertBlock()->getParent();

    llvm::BasicBlock *thenBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "then", function);
    llvm::BasicBlock *elseBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "else");
    llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "ifcont");

    ctx->llvm_ir_builder.CreateCondBr(conditionV, thenBB, elseBB);

    ctx->llvm_ir_builder.SetInsertPoint(thenBB);
    llvm::Value *ThenV = thenCodegen(ctx);
    if (!ThenV)
        return nullptr;
    ctx->llvm_ir_builder.CreateBr(mergeBB);
    thenBB = ctx->llvm_ir_builder.GetInsertBlock();

    llvm::Type *expected_type = ctx->expected_type;

    ctx->expected_type = ThenV->getType();

    function->getBasicBlockList().push_back(elseBB);
    ctx->llvm_ir_builder.SetInsertPoint(elseBB);
    llvm::Value *ElseV = elseCodegen(ctx);
    if (!ElseV)
        return nullptr;
    ctx->llvm_ir_builder.CreateBr(mergeBB);
    elseBB = ctx->llvm_ir_builder.GetInsertBlock();

    ctx->expected_type = expected_type;

    function->getBasicBlockList().push_back(mergeBB);
    ctx->llvm_ir_builder.SetInsertPoint(mergeBB);
    llvm::PHINode *PN =
            ctx->llvm_ir_builder.CreatePHI(ThenV->getType(), 2, "iftmp");
    PN->addIncoming(ThenV, thenBB);
    PN->addIncoming(ElseV, elseBB);

    return PN;
}

silicon::node_t silicon::ast::If::type() {
    return node_t::IF;
}

llvm::Value *silicon::ast::If::conditionCodegen(compiler::Context *ctx) {
    llvm::Value *value = condition->codegen(ctx);

    llvm::Type *type = value->getType();

    if (type->isVoidTy()) return ctx->bool_lit(false)->codegen(ctx);

    if (type->isArrayTy()) return ctx->bool_lit(true)->codegen(ctx);

    if (type->isIntegerTy(1))
        return ctx->llvm_ir_builder.CreateICmpEQ(value, ctx->bool_lit(true)->codegen(ctx));

    llvm::Type *expected_type = ctx->expected_type;

    ctx->expected_type = type;

    llvm::Value *v = ctx->num_lit("0")->codegen(ctx);

    ctx->expected_type = expected_type;

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateICmpNE(value, v);

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFCmpONE(value, v);

    fail_codegen("Unsupported condition");
}

llvm::Value *silicon::ast::If::thenCodegen(compiler::Context *ctx) {
    ctx->operator++();

    ctx->statements(then_statements);

    llvm::Value *value = ctx->codegen();

    ctx->operator--();

    return value;
}

llvm::Value *silicon::ast::If::elseCodegen(compiler::Context *ctx) {
    ctx->operator++();

    ctx->statements(else_statements);

    llvm::Value *value = ctx->codegen();

    ctx->operator--();

    return value;
}

silicon::ast::If *silicon::ast::If::setElse(std::vector<Node *> statements) {
    else_statements = std::move(statements);

    return this;
}
