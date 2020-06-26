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


#ifndef SILICON_BINARYOPERATION_H
#define SILICON_BINARYOPERATION_H


#include "Node.h"


namespace silicon::ast {

    class BinaryOperation : public Node {
    private:
        binary_operation_t op;
        Node *left, *right;

        BinaryOperation(binary_operation_t op, Node *left, Node *right);

        value_pair_t *parse_pair(compiler::Context *ctx);

        llvm::Value *assign(compiler::Context *ctx);

        llvm::Value *multiply(compiler::Context *ctx);

        llvm::Value *divide(compiler::Context *ctx);

        llvm::Value *remainder(compiler::Context *ctx);

        llvm::Value *add(compiler::Context *ctx);

        llvm::Value *sub(compiler::Context *ctx);

        llvm::Value *bw_xor(compiler::Context *ctx);

        llvm::Value *bw_and(compiler::Context *ctx);

        llvm::Value *bw_or(compiler::Context *ctx);

        llvm::Value *bw_left_shift(compiler::Context *ctx);

        llvm::Value *bw_right_shift(compiler::Context *ctx);

        llvm::Value *bw_u_right_shift(compiler::Context *ctx);

        llvm::Value *lt(compiler::Context *ctx);

        llvm::Value *lte(compiler::Context *ctx);

        llvm::Value *eq(compiler::Context *ctx);

        llvm::Value *ne(compiler::Context *ctx);

        llvm::Value *gte(compiler::Context *ctx);

        llvm::Value *gt(compiler::Context *ctx);

    public:
        static Node *create(compiler::Context *ctx, binary_operation_t op, Node *left, Node *right);

        llvm::Value *codegen(compiler::Context *ctx) override;

        node_t type() override;

    };

}


#endif //SILICON_BINARYOPERATION_H
