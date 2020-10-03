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


using namespace std;


namespace silicon::ast {

    using namespace compiler;

    class Type {
    private:
        function<llvm::Type *()> llvm_type;

        explicit Type(function<llvm::Type *()> llvm_type);

    protected:
        string loc;

    public:
        static Type *create(Context *ctx, llvm::Type *type);

        static Type *create(Context *ctx, const string &name);

        llvm::Type *codegen(Context *ctx);

        node_t type();

        bool type(node_t t);

        void fail_codegen(const string &error) noexcept __attribute__ ((__noreturn__));

    };

}

#define MOVE(V) move(V)


#endif //SILICON_TYPE_H
