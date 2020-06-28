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


#ifndef SILICON_NODE_H
#define SILICON_NODE_H


#include <parser/location.hh>
#include "llvm/IR/Value.h"
#include "utils.h"


namespace silicon::compiler {
    class Context;
}

namespace silicon::ast {

    class Node {
    protected:
        std::string loc;

    public:
        virtual ~Node() = default;

        void fail_codegen(const std::string &error);

        virtual llvm::Value *codegen(compiler::Context *ctx) = 0;

        virtual node_t type() = 0;

        bool type(node_t t);

    };

}


#endif //SILICON_NODE_H
