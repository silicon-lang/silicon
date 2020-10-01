//
//   Copyright 10/1/20 Ardalan Amini
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


#ifndef SILICON_TYPE_H
#define SILICON_TYPE_H


#include "Node.h"


namespace silicon::ast {

    class Type {
    private:
        std::function<llvm::Type *()> llvm_type;

        explicit Type(std::function<llvm::Type *()> llvm_type);

    protected:
        std::string loc;

    public:
        static Type *create(compiler::Context *ctx, llvm::Type *type);

        static Type *create(compiler::Context *ctx, const std::string &name);

        llvm::Type *codegen(compiler::Context *ctx);

        node_t type();

        bool type(node_t t);

        void fail_codegen(const std::string &error) noexcept __attribute__ ((__noreturn__));

    };

}

#define MOVE(V) std::move(V)


#endif //SILICON_TYPE_H
