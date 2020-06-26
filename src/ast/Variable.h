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


#ifndef SILICON_VARIABLE_H
#define SILICON_VARIABLE_H


#include "Node.h"


namespace silicon::ast {

    class Variable : public Node {
    private:
        std::string name;

        explicit Variable(std::string name);

    public:
        static Node *create(compiler::Context *ctx, const std::string& name);

        llvm::Value *codegen(compiler::Context *ctx) override;

        node_t type() override;

        std::string getName();

    };

}


#endif //SILICON_VARIABLE_H
