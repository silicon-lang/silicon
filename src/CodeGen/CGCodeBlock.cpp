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


#include "silicon/CodeGen/CGCodeBlock.h"


using namespace std;
using namespace llvm;
using namespace silicon::codegen;
using namespace silicon::parser::AST;


CGCodeBlock::CGCodeBlock(CodeBlock *node) : Node{node}, CodeBlock{node} {
}

Value *CGCodeBlock::codegen(Context *ctx) {
    parent = ctx->code_block;

    ctx->code_block = this;

    for (auto &statement: statements) {
        Value *value = dynamic_cast<CGNode *>(statement)->codegen(ctx);

        if (statement->is_node(node_t::RETURN)
            || statement->is_node(node_t::BREAK)
            || statement->is_node(node_t::CONTINUE)) {
            ctx->code_block = parent;

            return value;
        }
    }

    ctx->code_block = parent;

    return nullptr;
}

bool CGCodeBlock::allocated(const string &name) {
    if (variables[name]) return true;

    if (parent == nullptr) return false;

    return parent->allocated(name);
}

AllocaInst *CGCodeBlock::get_alloca(const string &name) {
    AllocaInst *variable = variables[name];

    if (variable) return variable;

    if (parent == nullptr) return nullptr;

    return parent->get_alloca(name);
}

Value *CGCodeBlock::alloc(Context *ctx, const string &name, Type *type) {
    if (this->allocated(name))
        fail("Variable <" + name + "> is already allocated");

    variables[name] = ctx->llvm_ir_builder.CreateAlloca(type, nullptr, name);

    unsigned bits = type->getScalarSizeInBits();

    if (bits > 0 && bits % 8 == 0) variables[name]->setAlignment(bits / 8);

    return variables[name];
}
