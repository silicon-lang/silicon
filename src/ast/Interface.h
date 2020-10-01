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


#ifndef SILICON_INTERFACE_H
#define SILICON_INTERFACE_H


#include "Node.h"
#include "Type.h"


namespace silicon::ast {

    class Interface : public Node {
    private:
        std::string name;
        std::vector<std::pair<std::string, ast::Type *>> properties;

        explicit Interface(std::string name, std::vector<std::pair<std::string, ast::Type *>> properties);

    public:
        static Interface *create(compiler::Context *ctx, std::string name, std::vector<std::pair<std::string, ast::Type *>> properties);

        llvm::Value *codegen(compiler::Context *ctx) override;

        node_t type() override;

    };

}


#endif //SILICON_INTERFACE_H
