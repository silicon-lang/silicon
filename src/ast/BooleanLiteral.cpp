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


#include "BooleanLiteral.h"
#include "compiler/Context.h"


silicon::ast::BooleanLiteral::BooleanLiteral(bool value) : value(value) {
}

silicon::ast::Node *silicon::ast::BooleanLiteral::create(silicon::compiler::Context *ctx, bool value) {
    auto *node = new BooleanLiteral(value);

    node->loc = parse_location(ctx->loc);

    return node;
}

llvm::Value *silicon::ast::BooleanLiteral::codegen(compiler::Context *ctx) {
    return ctx->llvm_ir_builder.getInt1(value);
}

silicon::node_t silicon::ast::BooleanLiteral::type() {
    return node_t::BOOLEAN_LIT;
}
