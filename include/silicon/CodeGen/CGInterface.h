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


#ifndef SILICON_CGINTERFACE_H
#define SILICON_CGINTERFACE_H


#include <string>
#include <vector>
#include "llvm/IR/Value.h"
#include "silicon/CodeGen/CGNode.h"
#include "silicon/CodeGen/CGType.h"
#include "silicon/CodeGen/Context.h"
#include "silicon/parser/AST/Interface.h"


namespace silicon::codegen {

    class CGInterface : public CGNode, public parser::AST::Interface {
    public:
        explicit CGInterface(parser::AST::Interface *node);

        llvm::Value *codegen(Context *ctx) override;

        uint64_t property_index(Context *ctx, const std::string &property);

        std::vector<std::pair<std::string, CGType *>> get_properties(Context *ctx);
    };

}


#endif //SILICON_CGINTERFACE_H
