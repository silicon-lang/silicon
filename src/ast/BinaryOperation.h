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

    using namespace compiler;

    class BinaryOperation : public Node {
    private:
        binary_operation_t op;
        Node *left, *right;

        BinaryOperation(binary_operation_t op, Node *left, Node *right);

        value_pair_t *parse_pair(Context *ctx);

        llvm::Value *assign(Context *ctx);

        llvm::Value *multiply(Context *ctx);

        llvm::Value *divide(Context *ctx);

        llvm::Value *remainder(Context *ctx);

        llvm::Value *add(Context *ctx);

        llvm::Value *sub(Context *ctx);

        llvm::Value *bw_xor(Context *ctx);

        llvm::Value *bw_and(Context *ctx);

        llvm::Value *bw_or(Context *ctx);

        llvm::Value *bw_left_shift(Context *ctx);

        llvm::Value *bw_right_shift(Context *ctx);

        llvm::Value *bw_u_right_shift(Context *ctx);

        llvm::Value *lt(Context *ctx);

        llvm::Value *lte(Context *ctx);

        llvm::Value *eq(Context *ctx);

        llvm::Value *ne(Context *ctx);

        llvm::Value *gte(Context *ctx);

        llvm::Value *gt(Context *ctx);

    public:
        static Node *create(Context *ctx, binary_operation_t op, Node *left, Node *right);

        llvm::Value *codegen(Context *ctx) override;

        node_t type() override;

    };

}


#endif //SILICON_BINARYOPERATION_H
