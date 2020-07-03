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


#include "While.h"
#include "compiler/Context.h"


silicon::ast::While::While(Node *condition, std::vector<Node *> body) : condition(condition), body(body) {
}

silicon::ast::While *silicon::ast::While::create(compiler::Context *ctx, Node *condition, std::vector<Node *> body) {
    auto *node = new While(condition, body);

    node->loc = parse_location(ctx->loc);

    return node;
}

llvm::Value *silicon::ast::While::codegen(silicon::compiler::Context *ctx) {
    if (!hasBody()) return nullptr;

    llvm::Value *conditionV = conditionCodegen(ctx);

    llvm::Function *function = ctx->llvm_ir_builder.GetInsertBlock()->getParent();

    llvm::BasicBlock *entryBB = ctx->llvm_ir_builder.GetInsertBlock();
    llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "loop");
    llvm::BasicBlock *afterBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "after_loop");

    ctx->llvm_ir_builder.CreateCondBr(conditionV, loopBB, afterBB);

    function->getBasicBlockList().push_back(loopBB);

    ctx->llvm_ir_builder.SetInsertPoint(loopBB);
    llvm::Value *thenV = bodyCodegen(ctx);
    if (!thenV) {
        conditionV = conditionCodegen(ctx);

        ctx->llvm_ir_builder.CreateCondBr(conditionV, loopBB, afterBB);
    }

    function->getBasicBlockList().push_back(afterBB);

    ctx->llvm_ir_builder.SetInsertPoint(afterBB);

    return nullptr;
}

silicon::node_t silicon::ast::While::type() {
    return node_t::WHILE;
}

llvm::Value *silicon::ast::While::conditionCodegen(compiler::Context *ctx) {
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

llvm::ReturnInst *silicon::ast::While::bodyCodegen(compiler::Context *ctx) {
    ctx->operator++();

    ctx->statements(body);

    llvm::ReturnInst *value = ctx->codegen();

    ctx->operator--();

    return value;
}

bool silicon::ast::While::hasBody() {
    return body.size() > 0;
}
