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


#ifndef SILICON_CGUNARYOPERATION_H
#define SILICON_CGUNARYOPERATION_H


#include "llvm/IR/Value.h"
#include "silicon/CodeGen/CGNode.h"
#include "silicon/CodeGen/Context.h"
#include "silicon/parser/AST/UnaryOperation.h"


namespace silicon::codegen {

    class CGUnaryOperation : public CGNode, public parser::AST::UnaryOperation {
    public:
        explicit CGUnaryOperation(parser::AST::UnaryOperation *node);

        llvm::Value *codegen(Context *ctx) override;

        std::string stringify_operator();

    protected:
        void unsupported_op(Context *ctx, llvm::Type *type) noexcept __attribute__ ((__noreturn__));

        llvm::Value *increment(Context *ctx);

        llvm::Value *decrement(Context *ctx);

        llvm::Value *negate(Context *ctx);

        llvm::Value *not_op(Context *ctx);
    };

}


#endif //SILICON_CGUNARYOPERATION_H
