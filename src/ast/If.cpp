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


using namespace std;
using namespace silicon;
using namespace ast;
using namespace compiler;


If::If(Node *condition, vector<Node *> then_statements, vector<Node *> else_statements) : condition(condition),
                                                                                          then_statements(
                                                                                                  MOVE(then_statements)),
                                                                                          else_statements(
                                                                                                  MOVE(else_statements)) {
}

If *If::create(Context *ctx, Node *condition, vector<Node *> then_statements, vector<Node *> else_statements) {
    auto *node = new If(condition, MOVE(then_statements), MOVE(else_statements));

    node->loc = parse_location(ctx->loc);

    return node;
}

llvm::Value *If::codegen(Context *ctx) {
    if (is_inline) return inlineCodegen(ctx);

    if (!hasThen() && !hasElse()) return nullptr;

    llvm::Value *conditionV = conditionCodegen(ctx);

    llvm::Function *function = ctx->llvm_ir_builder.GetInsertBlock()->getParent();

    llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "after_if");
    llvm::BasicBlock *thenBB = mergeBB;
    if (hasThen()) thenBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "then");
    llvm::BasicBlock *elseBB = mergeBB;
    if (hasElse()) elseBB = llvm::BasicBlock::Create(ctx->llvm_ctx, "else");

    ctx->llvm_ir_builder.CreateCondBr(conditionV, thenBB, elseBB);

    if (hasThen()) {
        function->getBasicBlockList().push_back(thenBB);

        ctx->llvm_ir_builder.SetInsertPoint(thenBB);
        llvm::Value *thenV = thenCodegen(ctx);
        if (!thenV) ctx->llvm_ir_builder.CreateBr(mergeBB);
    }

    if (hasElse()) {
        function->getBasicBlockList().push_back(elseBB);

        ctx->llvm_ir_builder.SetInsertPoint(elseBB);
        llvm::Value *elseV = elseCodegen(ctx);
        if (!elseV) ctx->llvm_ir_builder.CreateBr(mergeBB);
    }

    function->getBasicBlockList().push_back(mergeBB);

    ctx->llvm_ir_builder.SetInsertPoint(mergeBB);

    return nullptr;
}

node_t If::type() {
    return node_t::IF;
}

llvm::Value *If::inlineCodegen(Context *ctx) {
    llvm::Value *conditionV = conditionCodegen(ctx);

    llvm::Function *function = ctx->llvm_ir_builder.GetInsertBlock()->getParent();

    Node *Then = then_statements[0];
    Node *Else = else_statements[0];

    bool should_keep_then = !Then->type(node_t::BOOLEAN_LIT) && !Then->type(node_t::NUMBER_LIT);
    bool should_keep_else = !should_keep_then || (!Else->type(node_t::BOOLEAN_LIT) && !Else->type(node_t::NUMBER_LIT));

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
    if (!ThenV) fail_codegen("Error: Unrecognized <then> expression");
    if (should_keep_then) ctx->llvm_ir_builder.CreateBr(mergeBB);
    thenBB = ctx->llvm_ir_builder.GetInsertBlock();

    llvm::Type *expected_type = ctx->expected_type;

    ctx->expected_type = ThenV->getType();

    if (should_keep_else) {
        function->getBasicBlockList().push_back(elseBB);

        ctx->llvm_ir_builder.SetInsertPoint(elseBB);
    }
    llvm::Value *ElseV = Else->codegen(ctx);
    if (!ElseV) fail_codegen("Error: Unrecognized <else> expression");
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

llvm::Value *If::conditionCodegen(Context *ctx) {
    return ctx->def_cast(condition, ctx->bool_type())->codegen(ctx);
}

llvm::Value *If::thenCodegen(Context *ctx) {
    ctx->operator++();

    ctx->statements(then_statements);

    llvm::Value *value = ctx->codegen();

    ctx->operator--();

    return value;
}

llvm::Value *If::elseCodegen(Context *ctx) {
    ctx->operator++();

    ctx->statements(else_statements);

    llvm::Value *value = ctx->codegen();

    ctx->operator--();

    return value;
}

If *If::setElse(vector<Node *> statements) {
    else_statements = MOVE(statements);

    return this;
}

If *If::makeInline() {
    is_inline = true;

    return this;
}

bool If::hasThen() {
    return !then_statements.empty();
}

bool If::hasElse() {
    return !else_statements.empty();
}
