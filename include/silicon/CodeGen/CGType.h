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


#ifndef SILICON_CGTYPE_H
#define SILICON_CGTYPE_H


#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"
#include "silicon/CodeGen/CGNode.h"
#include "silicon/CodeGen/Context.h"
#include "silicon/parser/AST/Type.h"


namespace silicon::codegen {

    class CGType : public CGNode, public parser::AST::Type {
    public:
        explicit CGType(parser::AST::Type *node);

        llvm::Value *codegen(Context *ctx) override;

        llvm::Type *typegen(Context *ctx);
    };

}


#endif //SILICON_CGTYPE_H
