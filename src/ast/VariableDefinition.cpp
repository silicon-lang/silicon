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


#include "VariableDefinition.h"
#include "compiler/Context.h"


silicon::ast::VariableDefinition::VariableDefinition(std::string name, ast::Type *type) : name(MOVE(name)),
                                                                                          llvm_type(type) {
    if (!type) silicon_error("Argument <type> is required");
}

silicon::ast::Node *
silicon::ast::VariableDefinition::create(compiler::Context *ctx, const std::string &name, ast::Type *type) {
    if (!type) type = ctx->type(nullptr);

    auto *node = new VariableDefinition(name, type);

    node->loc = parse_location(ctx->loc);

    return node;
}

llvm::Value *silicon::ast::VariableDefinition::codegen(compiler::Context *ctx) {
    llvm::Type *t = llvm_type->codegen(ctx);

    if (!t) t = ctx->expected_type;

    return ctx->alloc(name, t);
}

silicon::node_t silicon::ast::VariableDefinition::type() {
    return node_t::VARIABLE_DEFINITION;
}

std::string silicon::ast::VariableDefinition::getName() {
    return name;
}

llvm::Type *silicon::ast::VariableDefinition::getLLVMType(compiler::Context *ctx) {
    return llvm_type->codegen(ctx);
}
