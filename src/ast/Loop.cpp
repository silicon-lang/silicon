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


#include "Loop.h"
#include "compiler/Context.h"


silicon::ast::Loop::Loop(std::vector<Node *> body) : body(body) {
}

silicon::ast::Loop *silicon::ast::Loop::create(compiler::Context *ctx, std::vector<Node *> body) {
    auto *node = new Loop(body);

    node->loc = parse_location(ctx->loc);

    return node;
}

llvm::Value *silicon::ast::Loop::codegen(compiler::Context *ctx) {
    llvm::Function *function = ctx->llvm_ir_builder.GetInsertBlock()->getParent();

    llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "loop");
    llvm::BasicBlock *afterBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "after_loop");

    ctx->llvm_ir_builder.CreateBr(loopBB);

    function->getBasicBlockList().push_back(loopBB);
    ctx->llvm_ir_builder.SetInsertPoint(loopBB);

    loop_points_t *loop_points = ctx->loop_points;
    ctx->loop_points = new loop_points_t();
    ctx->loop_points->break_point = afterBB;
    ctx->loop_points->continue_point = loopBB;

    llvm::Value *thenV = bodyCodegen(ctx);
    if (!thenV) ctx->llvm_ir_builder.CreateBr(loopBB);

    ctx->loop_points = loop_points;

    function->getBasicBlockList().push_back(afterBB);
    ctx->llvm_ir_builder.SetInsertPoint(afterBB);

    return nullptr;
}

silicon::node_t silicon::ast::Loop::type() {
    return node_t::LOOP;
}

llvm::Value *silicon::ast::Loop::bodyCodegen(compiler::Context *ctx) {
    ctx->operator++();

    ctx->statements(body);

    llvm::Value *value = ctx->codegen();

    ctx->operator--();

    return value;
}