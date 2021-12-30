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


#include <vector>
#include "silicon/CodeGen/CGPrototype.h"
#include "silicon/CodeGen/CGType.h"


using namespace std;
using namespace llvm;
using namespace silicon::codegen;
using namespace silicon::parser::AST;


CGPrototype::CGPrototype(Prototype *node) : Node{node}, Prototype{node} {
}

Value *CGPrototype::codegen(Context *ctx) {
    vector<string> names;
    vector<llvm::Type *> types;

    for (auto &argument: arguments) {
        names.push_back(argument.first);
        types.push_back(dynamic_cast<CGType *>(argument.second)->typegen(ctx));
    }

    llvm::Type *result_type = get_return_type(ctx);

    if (!result_type) result_type = ctx->void_type();

    FunctionType *function_type =
            FunctionType::get(result_type, types, is_variadic);

    Function::LinkageTypes linkage = Function::PrivateLinkage;

    if (name == "main" || is_extern || is_exported) linkage = llvm::Function::ExternalLinkage;

    llvm::Function *function =
            llvm::Function::Create(function_type, linkage, name, ctx->llvm_module.get());

    function->setDSOLocal(true);

    // Set names for all arguments.
    unsigned Idx = 0;
    for (auto &Arg: function->args()) Arg.setName(names[Idx++]);

    return function;
}

llvm::Type *CGPrototype::get_return_type(Context *ctx) {
    return dynamic_cast<CGType *>(return_type)->typegen(ctx);
}
