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


#ifndef SILICON_PROTOTYPE_H
#define SILICON_PROTOTYPE_H


#include "Node.h"
#include "llvm/IR/Function.h"


namespace silicon::ast {

    class Prototype : public Node {
    private:
        std::string name;

        std::vector<std::pair<std::string, llvm::Type *>> args;
        llvm::Type *return_type;

        llvm::Function::LinkageTypes linkage = llvm::Function::InternalLinkage;

        Prototype(std::string name, std::vector<std::pair<std::string, llvm::Type *>> args,
                  llvm::Type *return_type = nullptr);

    public:
        static Prototype *
        create(compiler::Context *ctx, const std::string &name, std::vector<std::pair<std::string, llvm::Type *>> args,
               llvm::Type *return_type = nullptr);

        llvm::Function *codegen(compiler::Context *ctx) override;

        node_t type() override;

        std::string getName();

        llvm::Type *getReturnType();

        Prototype *setReturnType(llvm::Type *type);

        Prototype *externalLinkage();

    };

}


#endif //SILICON_PROTOTYPE_H
