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
#include "While.h"
#include "compiler/Context.h"


silicon::ast::While::While(Node *condition, std::vector<Node *> body) : condition(condition), body(std::move(body)) {
}

silicon::ast::While *silicon::ast::While::create(compiler::Context *ctx, Node *condition, std::vector<Node *> body) {
    auto *node = new While(condition, std::move(body));

    node->loc = parse_location(ctx->loc);

    return node;
}

llvm::Value *silicon::ast::While::codegen(silicon::compiler::Context *ctx) {
    // TODO: can cause problem if the condition is something like "i--"
//    if (!hasBody()) return nullptr;

    llvm::Function *function = ctx->llvm_ir_builder.GetInsertBlock()->getParent();

    llvm::BasicBlock *conditionBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "loop_condition");
    llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "loop");
    llvm::BasicBlock *afterBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "after_loop");

    if (is_do_while) ctx->llvm_ir_builder.CreateBr(loopBB);
    else ctx->llvm_ir_builder.CreateBr(conditionBB);

    function->getBasicBlockList().push_back(conditionBB);
    ctx->llvm_ir_builder.SetInsertPoint(conditionBB);

    llvm::Value *conditionV = conditionCodegen(ctx);

    ctx->llvm_ir_builder.CreateCondBr(conditionV, loopBB, afterBB);

    function->getBasicBlockList().push_back(loopBB);
    ctx->llvm_ir_builder.SetInsertPoint(loopBB);

    llvm::Value *thenV = bodyCodegen(ctx);
    if (!thenV) ctx->llvm_ir_builder.CreateBr(conditionBB);

    function->getBasicBlockList().push_back(afterBB);
    ctx->llvm_ir_builder.SetInsertPoint(afterBB);

    return nullptr;
}

silicon::node_t silicon::ast::While::type() {
    return node_t::WHILE;
}

llvm::Value *silicon::ast::While::conditionCodegen(compiler::Context *ctx) {
    return ctx->def_cast(condition, ctx->bool_type())->codegen(ctx);
}

llvm::ReturnInst *silicon::ast::While::bodyCodegen(compiler::Context *ctx) {
    ctx->operator++();

    ctx->statements(body);

    llvm::ReturnInst *value = ctx->codegen();

    ctx->operator--();

    return value;
}

silicon::ast::While *silicon::ast::While::makeDoWhile() {
    is_do_while = true;

    return this;
}

bool silicon::ast::While::hasBody() {
    return body.size() > 0;
}
