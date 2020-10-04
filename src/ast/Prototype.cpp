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


#include <utility>
#include "Prototype.h"
#include "compiler/Context.h"


using namespace std;
using namespace silicon;
using namespace ast;
using namespace compiler;


Prototype::Prototype(const string &location, string name, vector<pair<string, Type *>> args, Type *return_type) : name(
        MOVE(name)), args(MOVE(args)), return_type(return_type) {
    if (!return_type) silicon_error("Argument <return_type> is required");

    this->location = location;
}

llvm::Function *Prototype::codegen(Context *ctx) {
    vector<string> names;
    vector<llvm::Type *> types;

    for (auto &arg : args) {
        names.push_back(arg.first);
        types.push_back(arg.second->codegen(ctx));
    }

    llvm::Type *result_type = return_type->codegen(ctx);

    if (!result_type) result_type = ctx->void_type();

    llvm::FunctionType *function_type =
            llvm::FunctionType::get(result_type, types, is_variadic);

    if (name == "main" || is_extern) linkage = llvm::Function::ExternalLinkage;

    llvm::Function *function =
            llvm::Function::Create(function_type, linkage, name, ctx->llvm_module.get());
    // Set names for all arguments.

    unsigned Idx = 0;
    for (auto &Arg : function->args()) Arg.setName(names[Idx++]);

    return function;
}

node_t Prototype::type() {
    return node_t::PROTOTYPE;
}

string Prototype::getName() {
    return name;
}

llvm::Type *Prototype::getReturnType(Context *ctx) {
    return return_type->codegen(ctx);
}

Prototype *Prototype::setReturnType(Context *ctx, llvm::Type *type) {
    return_type = ctx->type(type);

    return this;
}

Prototype *Prototype::externalLinkage() {
    linkage = llvm::Function::ExternalLinkage;

    return this;
}

Prototype *Prototype::makeExtern() {
    is_extern = true;

    return this;
}

Prototype *Prototype::makeVariadic() {
    is_variadic = true;

    return this;
}
