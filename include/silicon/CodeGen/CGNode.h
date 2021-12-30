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


#ifndef SILICON_CGNODE_H
#define SILICON_CGNODE_H


#include "llvm/IR/Value.h"
#include "silicon/CodeGen/Context.h"
#include "silicon/parser/AST/Node.h"


namespace silicon::codegen {

    class CGNode : virtual public parser::AST::Node {
    public:
        virtual llvm::Value *codegen(Context *ctx) = 0;
    };

}


#endif //SILICON_CGNODE_H