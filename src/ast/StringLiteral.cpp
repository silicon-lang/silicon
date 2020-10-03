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


using namespace std;
using namespace silicon;
using namespace ast;
using namespace compiler;


StringLiteral::StringLiteral(string value) {
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

Node *StringLiteral::create(Context *ctx, string value) {
    auto *node = new StringLiteral(MOVE(value));

    node->loc = parse_location(ctx->loc);

    return node;
}

llvm::Value *StringLiteral::codegen(Context *ctx) {
    string name = "string." + value;

    llvm::GlobalVariable *gs = ctx->llvm_module->getNamedGlobal(name);

    if (!gs) gs = ctx->llvm_ir_builder.CreateGlobalString(value, name);

    return ctx->llvm_ir_builder.CreateConstGEP2_64(gs->getValueType(), gs, 0, 0);
}

node_t StringLiteral::type() {
    return node_t::STRING_LIT;
}
