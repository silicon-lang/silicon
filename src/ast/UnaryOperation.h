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


#ifndef SILICON_UNARYOPERATION_H
#define SILICON_UNARYOPERATION_H


#include "Node.h"


namespace silicon::ast {

    class UnaryOperation : public Node {
    private:
        bool suffix;
        unary_operation_t op;
        Node *node;

        UnaryOperation(unary_operation_t op, Node *node, bool suffix = false);

        llvm::Value *increment(compiler::Context *ctx);

        llvm::Value *decrement(compiler::Context *ctx);

        llvm::Value *negate(compiler::Context *ctx);

        llvm::Value *not_op(compiler::Context *ctx);

    public:
        static Node *create(compiler::Context *ctx, unary_operation_t op, Node *node, bool suffix = false);

        llvm::Value *codegen(compiler::Context *ctx) override;

        node_t type() override;

    };

}


#endif //SILICON_UNARYOPERATION_H
