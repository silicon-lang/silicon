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
    llvm::BasicBlock *conditionBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "loop_condition");
    llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "loop");
    llvm::BasicBlock *stepperBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "loop_stepper");
    llvm::BasicBlock *afterBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "after_loop");

    ctx->llvm_ir_builder.CreateBr(preBB);
    function->getBasicBlockList().push_back(preBB);
    ctx->llvm_ir_builder.SetInsertPoint(preBB);

    ctx->operator++();

    definitionCodegen(ctx);

    ctx->llvm_ir_builder.CreateBr(conditionBB);
    function->getBasicBlockList().push_back(conditionBB);
    ctx->llvm_ir_builder.SetInsertPoint(conditionBB);

    llvm::Value *conditionV = conditionCodegen(ctx);

    ctx->llvm_ir_builder.CreateCondBr(conditionV, loopBB, afterBB);

    function->getBasicBlockList().push_back(loopBB);
    ctx->llvm_ir_builder.SetInsertPoint(loopBB);

    llvm::Value *thenV = bodyCodegen(ctx);
    if (!thenV) {
        ctx->llvm_ir_builder.CreateBr(stepperBB);

        function->getBasicBlockList().push_back(stepperBB);
        ctx->llvm_ir_builder.SetInsertPoint(stepperBB);

        stepperCodegen(ctx);

        ctx->llvm_ir_builder.CreateBr(conditionBB);
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
    return ctx->def_cast(condition, ctx->bool_type())->codegen(ctx);
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
