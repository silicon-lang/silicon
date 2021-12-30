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


#ifndef SILICON_CGCODEBLOCK_H
#define SILICON_CGCODEBLOCK_H


#include <map>
#include <string>
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "silicon/CodeGen/CGNode.h"
#include "silicon/CodeGen/Context.h"
#include "silicon/parser/AST/CodeBlock.h"


namespace silicon::codegen {

    class CGCodeBlock : public CGNode, public parser::AST::CodeBlock {
    protected:
        CGCodeBlock *parent = nullptr;

        std::map<std::string, llvm::AllocaInst *> variables;

    public:
        explicit CGCodeBlock(parser::AST::CodeBlock *node);

        llvm::Value *codegen(Context *ctx) override;

        bool allocated(const std::string &name);

        llvm::AllocaInst *get_alloca(const std::string &name);

        llvm::Value *alloc(Context *ctx, const std::string &name, llvm::Type *type);

        friend class Context;
    };

}


#endif //SILICON_CGCODEBLOCK_H
