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


#include "llvm/IR/Instructions.h"
#include "silicon/CodeGen/CGFor.h"


using namespace llvm;
using namespace silicon::codegen;
using namespace silicon::parser::AST;


CGFor::CGFor(For *node) : Node{node}, For{node} {
}

Value *CGFor::codegen(Context *ctx) {
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

    loop_points_t *loop_points = ctx->loop_points;
    ctx->loop_points = new loop_points_t();
    ctx->loop_points->break_point = afterBB;
    ctx->loop_points->continue_point = stepperBB;

    llvm::Value *thenV = bodyCodegen(ctx);
    if (!thenV) ctx->llvm_ir_builder.CreateBr(stepperBB);

    ctx->loop_points = loop_points;

    function->getBasicBlockList().push_back(stepperBB);
    ctx->llvm_ir_builder.SetInsertPoint(stepperBB);

    stepperCodegen(ctx);

    ctx->llvm_ir_builder.CreateBr(conditionBB);

    ctx->operator--();

    function->getBasicBlockList().push_back(afterBB);
    ctx->llvm_ir_builder.SetInsertPoint(afterBB);

    return nullptr;
}

Value *CGFor::definitionCodegen(Context *ctx) {
    return dynamic_cast<CGNode *>(definition)->codegen(ctx);
}

Value *CGFor::conditionCodegen(Context *ctx) {
    return ctx->cast_type(dynamic_cast<CGNode *>(condition)->codegen(ctx), ctx->bool_type());
}

Value *CGFor::stepperCodegen(Context *ctx) {
    return dynamic_cast<CGNode *>(stepper)->codegen(ctx);
}

Value *CGFor::bodyCodegen(Context *ctx) {
    return dynamic_cast<CGNode *>(body)->codegen(ctx);
}
