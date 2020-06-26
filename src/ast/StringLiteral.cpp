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


#include "StringLiteral.h"
#include "compiler/Context.h"
#include <utility>


silicon::ast::StringLiteral::StringLiteral(std::string value) {
    value = replace_all(value, "\\t", "\t");
    value = replace_all(value, "\\v", "\v");
    value = replace_all(value, "\\0", "\0");
    value = replace_all(value, "\\b", "\b");
    value = replace_all(value, "\\f", "\f");
    value = replace_all(value, "\\n", "\n");
    value = replace_all(value, "\\r", "\r");
    value = replace_all(value, "\\'", "\'");
    value = replace_all(value, "\\\"", "\"");
    value = replace_all(value, "\\\\", "\\");

    this->value = value;
}

silicon::ast::Node *silicon::ast::StringLiteral::create(compiler::Context *ctx, std::string value) {
    auto *node = new StringLiteral(std::move(value));

    node->loc = parse_location(ctx->loc);

    return node;
}

llvm::Value *silicon::ast::StringLiteral::codegen(compiler::Context *ctx) {
    return ctx->llvm_ir_builder.CreateGlobalString(value);
}

silicon::node_t silicon::ast::StringLiteral::type() {
    return node_t::STRING_LIT;
}
