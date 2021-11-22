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


#ifndef SILICON_CGIf_H
#define SILICON_CGIf_H


#include "llvm/IR/Value.h"
#include "silicon/CodeGen/CGNode.h"
#include "silicon/CodeGen/Context.h"
#include "silicon/parser/AST/If.h"


namespace silicon::codegen {

    class CGIf : public CGNode, public parser::AST::If {
    protected:
        bool has_then();

        bool has_else();

        llvm::Value *inline_codegen(Context *ctx);

        llvm::Value *condition_codegen(Context *ctx);

        llvm::Value *then_codegen(Context *ctx);

        llvm::Value *else_codegen(Context *ctx);

    public:
        explicit CGIf(parser::AST::If *node);

        llvm::Value *codegen(Context *ctx) override;
    };

}


#endif //SILICON_CGIf_H
