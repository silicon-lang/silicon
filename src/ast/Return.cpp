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


#include "Return.h"
#include "compiler/Context.h"


silicon::ast::Return::Return(Node *value) : value(value) {
}

silicon::ast::Node *silicon::ast::Return::create(compiler::Context *ctx, Node *value) {
    auto *node = new Return(value);

    node->loc = parse_location(ctx->loc);

    return node;
}

llvm::Value *silicon::ast::Return::codegen(compiler::Context *ctx) {
    llvm::Value *ret = value->codegen(ctx);

    if (ctx->expected_type && !compare_types(ret->getType(), ctx->expected_type)) {
        fail_codegen(
                "TypeError: Expected function to return <"
                + parse_type(ctx->expected_type)
                + ">, got <"
                + parse_type(ret->getType())
                + "> instead."
        );
    }

    return ctx->llvm_ir_builder.CreateRet(ret);
}

silicon::node_t silicon::ast::Return::type() {
    return node_t::RETURN;
}
