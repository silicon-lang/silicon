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


silicon::ast::Prototype::Prototype(std::string name, std::vector<std::pair<std::string, llvm::Type *>> args,
                                   llvm::Type *return_type) : name(std::move(name)), args(std::move(args)),
                                                              return_type(return_type) {
}

silicon::ast::Prototype *silicon::ast::Prototype::create(compiler::Context *ctx, const std::string &name,
                                                         std::vector<std::pair<std::string, llvm::Type *>> args,
                                                         llvm::Type *return_type) {
    auto *node = new Prototype(name, std::move(args), return_type);

    node->loc = parse_location(ctx->loc);

    return node;
}

llvm::Function *silicon::ast::Prototype::codegen(compiler::Context *ctx) {
    std::vector<std::string> names;
    std::vector<llvm::Type *> types;

    for (auto &arg : args) {
        names.push_back(arg.first);
        types.push_back(arg.second);
    }

    llvm::Type *result_type = return_type;

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

silicon::node_t silicon::ast::Prototype::type() {
    return node_t::PROTOTYPE;
}

std::string silicon::ast::Prototype::getName() {
    return name;
}

llvm::Type *silicon::ast::Prototype::getReturnType() {
    return return_type;
}

silicon::ast::Prototype *silicon::ast::Prototype::setReturnType(llvm::Type *type) {
    return_type = type;

    return this;
}

silicon::ast::Prototype *silicon::ast::Prototype::externalLinkage() {
    linkage = llvm::Function::ExternalLinkage;

    return this;
}

silicon::ast::Prototype *silicon::ast::Prototype::makeExtern() {
    is_extern = true;

    return this;
}

silicon::ast::Prototype *silicon::ast::Prototype::makeVariadic() {
    is_variadic = true;

    return this;
}
