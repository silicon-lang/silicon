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


#ifndef SILICON_FOR_H
#define SILICON_FOR_H


#include <llvm/IR/Instructions.h>
#include "Node.h"


namespace silicon::ast {

    class For : public Node {
    private:
        Node *definition;
        Node *condition;
        Node *stepper;
        std::vector<Node *> body;

        For(Node *definition, Node *condition, Node *stepper, std::vector<Node *> body);

        llvm::Value *definitionCodegen(compiler::Context *ctx);

        llvm::Value *conditionCodegen(compiler::Context *ctx);

        llvm::Value *stepperCodegen(compiler::Context *ctx);

        llvm::Value *bodyCodegen(compiler::Context *ctx);

    public:
        static For *
        create(compiler::Context *ctx, Node *definition, Node *condition, Node *stepper, std::vector<Node *> body);

        llvm::Value *codegen(compiler::Context *ctx) override;

        node_t type() override;

    };

}


#endif //SILICON_FOR_H
