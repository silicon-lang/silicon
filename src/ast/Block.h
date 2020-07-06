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


#ifndef SILICON_BLOCK_H
#define SILICON_BLOCK_H


#include <vector>
#include <map>
#include <llvm/IR/Instructions.h>
#include "Node.h"


namespace silicon::ast {

    class Block : public Node {
    private:
        Block *parent;

        std::vector<Node *> statements;

        std::map<std::string, llvm::AllocaInst *> variables;

        explicit Block(Block *parent = nullptr);

    public:
        static Block *create(compiler::Context *ctx, Block *parent = nullptr);

        llvm::Value *codegen(compiler::Context *ctx) override;

        node_t type() override;

        Block *getParent();

        Block *setStatements(const std::vector<Node *> &nodes);

        Block *push(Node *statement);

        bool allocated(const std::string &name);

        llvm::AllocaInst *get_alloca(const std::string &name);

        llvm::Value *alloc(compiler::Context *ctx, const std::string &name, llvm::Type *type);

        llvm::Value *store(compiler::Context *ctx, const std::string &name, llvm::Value *value);

        llvm::Value *load(compiler::Context *ctx, const std::string &name);

    };

}


#endif //SILICON_BLOCK_H
