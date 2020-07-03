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


#include "For.h"
#include "compiler/Context.h"


silicon::ast::For::For(Node *definition, Node *condition, Node *stepper, std::vector<Node *> body) : definition(
        definition), condition(condition), stepper(stepper), body(body) {
}

silicon::ast::For *
silicon::ast::For::create(compiler::Context *ctx, Node *definition, Node *condition, Node *stepper,
                          std::vector<Node *> body) {
    auto *node = new For(definition, condition, stepper, body);

    node->loc = parse_location(ctx->loc);

    return node;
}

llvm::Value *silicon::ast::For::codegen(compiler::Context *ctx) {
    llvm::Function *function = ctx->llvm_ir_builder.GetInsertBlock()->getParent();

    llvm::BasicBlock *preBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "pre_loop");
    llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "loop");
    llvm::BasicBlock *afterBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "after_loop");

    ctx->llvm_ir_builder.CreateBr(preBB);

    function->getBasicBlockList().push_back(preBB);

    ctx->llvm_ir_builder.SetInsertPoint(preBB);

    ctx->operator++();

    definitionCodegen(ctx);

    llvm::Value *conditionV = conditionCodegen(ctx);

    ctx->llvm_ir_builder.CreateCondBr(conditionV, loopBB, afterBB);

    function->getBasicBlockList().push_back(loopBB);

    ctx->llvm_ir_builder.SetInsertPoint(loopBB);
    llvm::Value *thenV = bodyCodegen(ctx);
    if (!thenV) {
        stepperCodegen(ctx);

        conditionV = conditionCodegen(ctx);

        ctx->llvm_ir_builder.CreateCondBr(conditionV, loopBB, afterBB);
    }

    ctx->operator--();

    function->getBasicBlockList().push_back(afterBB);

    ctx->llvm_ir_builder.SetInsertPoint(afterBB);

    return nullptr;
}

silicon::node_t silicon::ast::For::type() {
    return node_t::FOR;
}

llvm::Value *silicon::ast::For::definitionCodegen(compiler::Context *ctx) {
    return definition->codegen(ctx);
}

llvm::Value *silicon::ast::For::conditionCodegen(compiler::Context *ctx) {
    llvm::Value *value = condition->codegen(ctx);

    llvm::Type *type = value->getType();

    if (type->isIntegerTy(1)) return value;

    if (type->isVoidTy()) return ctx->bool_lit(false)->codegen(ctx);

    if (type->isArrayTy()) return ctx->bool_lit(true)->codegen(ctx);

    llvm::Type *expected_type = ctx->expected_type;

    ctx->expected_type = type;

    llvm::Value *v = ctx->num_lit("0")->codegen(ctx);

    ctx->expected_type = expected_type;

    if (type->isIntegerTy()) return ctx->llvm_ir_builder.CreateICmpNE(value, v);

    if (type->isFloatingPointTy()) return ctx->llvm_ir_builder.CreateFCmpONE(value, v);

    fail_codegen("Error: Unsupported condition");
}

llvm::Value *silicon::ast::For::stepperCodegen(compiler::Context *ctx) {
    return stepper->codegen(ctx);
}

llvm::ReturnInst *silicon::ast::For::bodyCodegen(compiler::Context *ctx) {
    ctx->operator++();

    ctx->statements(body);

    llvm::ReturnInst *value = ctx->codegen();

    ctx->operator--();

    return value;
}
