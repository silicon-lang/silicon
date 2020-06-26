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


#include "Variable.h"
#include "compiler/Context.h"


silicon::ast::Variable::Variable(std::string name) : name(std::move(name)) {
}

silicon::ast::Node *silicon::ast::Variable::create(compiler::Context *ctx, const std::string &name) {
    auto *node = new Variable(name);

    node->loc = parse_location(ctx->loc);

    return node;
}

llvm::Value *silicon::ast::Variable::codegen(compiler::Context *ctx) {
    return ctx->load(name);
}

silicon::node_t silicon::ast::Variable::type() {
    return node_t::VARIABLE;
}

std::string silicon::ast::Variable::getName() {
    return name;
}
