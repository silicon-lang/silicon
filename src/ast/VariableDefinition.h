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


#ifndef SILICON_VARIABLEDEFINITION_H
#define SILICON_VARIABLEDEFINITION_H


#include "Node.h"


namespace silicon::ast {

    class VariableDefinition : public Node {
    private:
        std::string name;
        llvm::Type *llvm_type;

        explicit VariableDefinition(std::string name, llvm::Type *type = nullptr);

    public:
        static Node *create(compiler::Context *ctx, const std::string& name, llvm::Type *type = nullptr);

        llvm::Value *codegen(compiler::Context *ctx) override;

        node_t type() override;

        std::string getName();

        llvm::Type *getLLVMType();

    };

}


#endif //SILICON_VARIABLEDEFINITION_H
