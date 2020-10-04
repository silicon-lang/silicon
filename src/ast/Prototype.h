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


#ifndef SILICON_PROTOTYPE_H
#define SILICON_PROTOTYPE_H


#include "Node.h"
#include "Type.h"
#include "llvm/IR/Function.h"


using namespace std;


namespace silicon::ast {

    using namespace compiler;

    class Prototype : public Node {
    private:
        string name;

        vector<pair<string, Type *>> args;
        Type *return_type;

        bool is_extern = false;
        bool is_variadic = false;

        llvm::Function::LinkageTypes linkage = llvm::Function::PrivateLinkage;

    public:
        Prototype(const string &location, string name, vector<pair<string, Type *>> args, Type *return_type);

        llvm::Function *codegen(Context *ctx) override;

        node_t type() override;

        string getName();

        llvm::Type *getReturnType(Context *ctx);

        Prototype *setReturnType(Context *ctx, llvm::Type *type);

        Prototype *externalLinkage();

        Prototype *makeExtern();

        Prototype *makeVariadic();

    };

}


#endif //SILICON_PROTOTYPE_H
