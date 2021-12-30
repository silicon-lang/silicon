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


#include "silicon/CodeGen/CGLoop.h"


using namespace llvm;
using namespace silicon::codegen;
using namespace silicon::parser::AST;


CGLoop::CGLoop(Loop *node) : Node{node}, Loop{node} {
}

Value *CGLoop::codegen(Context *ctx) {
    Function *function = ctx->llvm_ir_builder.GetInsertBlock()->getParent();

    BasicBlock *loopBB = BasicBlock::Create(ctx->llvm_ctx, "loop");
    BasicBlock *afterBB = BasicBlock::Create(ctx->llvm_ctx, "after_loop");

    ctx->llvm_ir_builder.CreateBr(loopBB);

    function->getBasicBlockList().push_back(loopBB);
    ctx->llvm_ir_builder.SetInsertPoint(loopBB);

    loop_points_t *loop_points = ctx->loop_points;
    ctx->loop_points = new loop_points_t();
    ctx->loop_points->break_point = afterBB;
    ctx->loop_points->continue_point = loopBB;

    Value *thenV = body_codegen(ctx);
    if (!thenV) ctx->llvm_ir_builder.CreateBr(loopBB);

    ctx->loop_points = loop_points;

    function->getBasicBlockList().push_back(afterBB);
    ctx->llvm_ir_builder.SetInsertPoint(afterBB);

    return nullptr;
}

Value *CGLoop::body_codegen(Context *ctx) {
    return dynamic_cast<CGNode *>(body)->codegen(ctx);
}
