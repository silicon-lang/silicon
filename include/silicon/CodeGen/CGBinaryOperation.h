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


#ifndef SILICON_CGBINARYOPERATION_H
#define SILICON_CGBINARYOPERATION_H


#include <string>
#include "llvm/IR/Value.h"
#include "silicon/CodeGen/CGNode.h"
#include "silicon/CodeGen/Context.h"
#include "silicon/parser/AST/BinaryOperation.h"


namespace silicon::codegen {

    struct value_pair_t {
        value_pair_t() = default;

        llvm::Value *left = nullptr;
        llvm::Value *right = nullptr;
    };

    class CGBinaryOperation : public CGNode, public parser::AST::BinaryOperation {
    public:
        explicit CGBinaryOperation(parser::AST::BinaryOperation *node);

        llvm::Value *codegen(Context *ctx) override;

        std::string stringify_operator();

    protected:
        void unsupported_op(Context *ctx, llvm::Type *t1, llvm::Type *t2) noexcept __attribute__ ((__noreturn__));

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

        llvm::Value *cast(Context *ctx);
    };

}


#endif //SILICON_CGBINARYOPERATION_H
