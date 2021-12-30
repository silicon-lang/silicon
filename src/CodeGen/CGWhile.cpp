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


#include "silicon/CodeGen/CGWhile.h"


using namespace llvm;
using namespace silicon::codegen;
using namespace silicon::parser::AST;


CGWhile::CGWhile(While *node) : Node{node}, While{node} {
}

Value *CGWhile::codegen(Context *ctx) {
    // TODO: can cause problem if the condition is something like "i--"
//    if (!hasBody()) return nullptr;

    Function *function = ctx->llvm_ir_builder.GetInsertBlock()->getParent();

    BasicBlock *conditionBB = BasicBlock::Create(ctx->llvm_ctx, "loop_condition");
    BasicBlock *loopBB = BasicBlock::Create(ctx->llvm_ctx, "loop");
    BasicBlock *afterBB = BasicBlock::Create(ctx->llvm_ctx, "after_loop");

    if (is_do_while) ctx->llvm_ir_builder.CreateBr(loopBB);
    else ctx->llvm_ir_builder.CreateBr(conditionBB);

    function->getBasicBlockList().push_back(conditionBB);
    ctx->llvm_ir_builder.SetInsertPoint(conditionBB);

    Value *conditionV = condition_codegen(ctx);

    ctx->llvm_ir_builder.CreateCondBr(conditionV, loopBB, afterBB);

    function->getBasicBlockList().push_back(loopBB);
    ctx->llvm_ir_builder.SetInsertPoint(loopBB);

    loop_points_t *loop_points = ctx->loop_points;
    ctx->loop_points = new loop_points_t();
    ctx->loop_points->break_point = afterBB;
    ctx->loop_points->continue_point = conditionBB;

    Value *thenV = body_codegen(ctx);
    if (!thenV) ctx->llvm_ir_builder.CreateBr(conditionBB);

    ctx->loop_points = loop_points;

    function->getBasicBlockList().push_back(afterBB);
    ctx->llvm_ir_builder.SetInsertPoint(afterBB);

    return nullptr;
}

bool CGWhile::has_body() {
    return body != nullptr;
}

Value *CGWhile::condition_codegen(Context *ctx) {
    return ctx->cast_type(dynamic_cast<CGNode *>(condition)->codegen(ctx), ctx->bool_type());
}

Value *CGWhile::body_codegen(Context *ctx) {
    return dynamic_cast<CGNode *>(body)->codegen(ctx);
}
