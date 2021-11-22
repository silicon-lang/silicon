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


#include "silicon/CodeGen/CGIf.h"


using namespace llvm;
using namespace silicon::codegen;
using namespace silicon::parser::AST;


CGIf::CGIf(If *node) : Node{node}, If{node} {
}

Value *CGIf::codegen(Context *ctx) {
    if (is_inline) return inline_codegen(ctx);

    if (!has_then() && !has_else()) return nullptr;

    llvm::Value *conditionV = condition_codegen(ctx);

    llvm::Function *function = ctx->llvm_ir_builder.GetInsertBlock()->getParent();

    llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "after_if");
    llvm::BasicBlock *thenBB = mergeBB;
    if (has_then()) thenBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "then");
    llvm::BasicBlock *elseBB = mergeBB;
    if (has_else()) elseBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "else");

    ctx->llvm_ir_builder.CreateCondBr(conditionV, thenBB, elseBB);

    if (has_then()) {
        function->getBasicBlockList().push_back(thenBB);

        ctx->llvm_ir_builder.SetInsertPoint(thenBB);
        llvm::Value *thenV = then_codegen(ctx);
        if (!thenV) ctx->llvm_ir_builder.CreateBr(mergeBB);
    }

    if (has_else()) {
        function->getBasicBlockList().push_back(elseBB);

        ctx->llvm_ir_builder.SetInsertPoint(elseBB);
        llvm::Value *elseV = else_codegen(ctx);
        if (!elseV) ctx->llvm_ir_builder.CreateBr(mergeBB);
    }

    function->getBasicBlockList().push_back(mergeBB);

    ctx->llvm_ir_builder.SetInsertPoint(mergeBB);

    return nullptr;
}

bool CGIf::has_then() {
    return then_statements != nullptr;
}

bool CGIf::has_else() {
    return else_statements != nullptr;
}

Value *CGIf::inline_codegen(Context *ctx) {
    llvm::Value *conditionV = condition_codegen(ctx);

    llvm::Function *function = ctx->llvm_ir_builder.GetInsertBlock()->getParent();

    auto *Then = dynamic_cast<CGNode *>(then_statements);
    auto *Else = dynamic_cast<CGNode *>(else_statements);

    bool should_keep_then = !Then->is_node(node_t::BOOLEAN_LIT) && !Then->is_node(node_t::NUMBER_LIT);
    bool should_keep_else = !should_keep_then || (!Else->is_node(node_t::BOOLEAN_LIT) && !Else->is_node(node_t::NUMBER_LIT));

    llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "after_if");
    llvm::BasicBlock *thenBB = mergeBB;
    if (should_keep_then) thenBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "then");
    llvm::BasicBlock *elseBB = mergeBB;
    if (should_keep_else) elseBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "else");

    ctx->llvm_ir_builder.CreateCondBr(conditionV, thenBB, elseBB);

    if (should_keep_then) {
        function->getBasicBlockList().push_back(thenBB);

        ctx->llvm_ir_builder.SetInsertPoint(thenBB);
    }
    llvm::Value *ThenV = Then->codegen(ctx);
    if (!ThenV) fail("Error: Unrecognized <then> expression");
    if (should_keep_then) ctx->llvm_ir_builder.CreateBr(mergeBB);
    thenBB = ctx->llvm_ir_builder.GetInsertBlock();

    llvm::Type *expected_type = ctx->expected_type;

    ctx->expected_type = ThenV->getType();

    if (should_keep_else) {
        function->getBasicBlockList().push_back(elseBB);

        ctx->llvm_ir_builder.SetInsertPoint(elseBB);
    }
    llvm::Value *ElseV = Else->codegen(ctx);
    if (!ElseV) fail("Error: Unrecognized <else> expression");
    if (should_keep_else) ctx->llvm_ir_builder.CreateBr(mergeBB);
    elseBB = ctx->llvm_ir_builder.GetInsertBlock();

    ctx->expected_type = expected_type;

    function->getBasicBlockList().push_back(mergeBB);

    ctx->llvm_ir_builder.SetInsertPoint(mergeBB);

    llvm::PHINode *PN =
            ctx->llvm_ir_builder.CreatePHI(ThenV->getType(), 2, "if_result");
    PN->addIncoming(ThenV, thenBB);
    PN->addIncoming(ElseV, elseBB);

    return PN;
}

llvm::Value *CGIf::condition_codegen(Context *ctx) {
    return ctx->cast_type(dynamic_cast<CGNode *>(condition)->codegen(ctx), ctx->bool_type());
}

llvm::Value *CGIf::then_codegen(Context *ctx) {
    return dynamic_cast<CGNode *>(then_statements)->codegen(ctx);
}

llvm::Value *CGIf::else_codegen(Context *ctx) {
    return dynamic_cast<CGNode *>(else_statements)->codegen(ctx);
}
