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


using namespace std;
using namespace silicon;
using namespace ast;
using namespace compiler;


Block::Block(Block *parent) : parent(parent) {
}

Block *Block::create(Context *ctx, ast::Block *parent) {
    auto *node = new Block(parent);

    node->loc = parse_location(ctx->loc);

//    if (parent) parent->push(node);

    return node;
}

llvm::Value *Block::codegen(Context *ctx) {
//    llvm::Type *expected_type = ctx->expected_type;

    for (auto &statement : statements) {
        if (statement->type(node_t::RETURN)
            || statement->type(node_t::BREAK)
            || statement->type(node_t::CONTINUE)) {
            // TODO: maybe throw a warning if there's more statements after the return statement?
            return statement->codegen(ctx);
        }

//        ctx->expected_type = nullptr;

        statement->codegen(ctx);

//        ctx->expected_type = expected_type;
    }

    return nullptr;
}

node_t Block::type() {
    return node_t::BLOCK;
}

Block *Block::getParent() {
    return parent;
}

Block *Block::setStatements(const vector<Node *> &nodes) {
    statements = nodes;

    return this;
}

Block *Block::push(Node *statement) {
    statements.push_back(statement);

    return this;
}

bool Block::allocated(const string &name) {
    if (variables[name]) return true;

    if (parent == nullptr) return false;

    return parent->allocated(name);
}

llvm::AllocaInst *Block::get_alloca(const string &name) {
    llvm::AllocaInst *variable = variables[name];

    if (variable) return variable;

    if (parent == nullptr) return nullptr;

    return parent->get_alloca(name);
}

llvm::Value *Block::alloc(Context *ctx, const string &name, llvm::Type *type) {
    if (this->allocated(name))
        fail_codegen("Variable <" + name + "> is already allocated");

    variables[name] = ctx->llvm_ir_builder.CreateAlloca(type, nullptr, name);

    unsigned bits = type->getScalarSizeInBits();

    if (bits > 0 && bits % 8 == 0) variables[name]->setAlignment(bits / 8);

    return variables[name];
}
