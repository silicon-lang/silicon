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


#include "Block.h"
#include "compiler/Context.h"


silicon::ast::Block::Block(silicon::ast::Block *parent) : parent(parent) {
}

silicon::ast::Block *silicon::ast::Block::create(compiler::Context *ctx, ast::Block *parent) {
    auto *node = new Block(parent);

    node->loc = parse_location(ctx->loc);

//    if (parent) parent->push(node);

    return node;
}

llvm::Value *silicon::ast::Block::codegen(compiler::Context *ctx) {
    llvm::Value *value = nullptr;

    llvm::Type *expected_type = ctx->expected_type;

    size_t size = statements.size();
    size_t idx = 0;

    for (auto &statement : statements) {
        if (idx++ != size - 1
            && !statement->type(node_t::RETURN))
            ctx->expected_type = nullptr;

        value = statement->codegen(ctx);

        ctx->expected_type = expected_type;
    }

    // TODO: return expression
    return value;
}

silicon::node_t silicon::ast::Block::type() {
    return node_t::BLOCK;
}

silicon::ast::Block *silicon::ast::Block::getParent() {
    return parent;
}

silicon::ast::Block *silicon::ast::Block::setStatements(const std::vector<Node *> &nodes) {
    statements = nodes;

    return this;
}

silicon::ast::Block *silicon::ast::Block::push(Node *statement) {
    statements.push_back(statement);

    return this;
}

bool silicon::ast::Block::allocated(const std::string &name) {
    if (variables[name]) return true;

    if (parent == nullptr) return false;

    return parent->allocated(name);
}

llvm::AllocaInst *silicon::ast::Block::get_alloca(const std::string &name) {
    llvm::AllocaInst *variable = variables[name];

    if (variable) return variable;

    if (parent == nullptr) return nullptr;

    return parent->get_alloca(name);
}

llvm::Value *silicon::ast::Block::alloc(compiler::Context *ctx, const std::string &name, llvm::Type *type) {
    if (this->allocated(name))
        fail_codegen("Variable <" + name + "> is already allocated");

    return variables[name] = ctx->llvm_ir_builder.CreateAlloca(type, nullptr, name);
}

llvm::Value *silicon::ast::Block::store(compiler::Context *ctx, const std::string &name, llvm::Value *value) {
    auto *variable = this->get_alloca(name);

    if (!variable)
        fail_codegen("Variable <" + name + "> is not allocated yet");

    return ctx->llvm_ir_builder.CreateStore(value, variable);
}

llvm::Value *silicon::ast::Block::load(compiler::Context *ctx, const std::string &name) {
    auto *variable = this->get_alloca(name);

    if (!variable)
        fail_codegen("Variable <" + name + "> is not allocated yet");

    return ctx->llvm_ir_builder.CreateLoad(variable, name);
}
