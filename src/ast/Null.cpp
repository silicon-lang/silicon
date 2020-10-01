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


#include "Null.h"
#include "compiler/Context.h"


silicon::ast::Null::Null(ast::Type *llvm_type) : llvm_type(llvm_type) {
    if (!llvm_type) silicon_error("Argument <llvm_type> is required");
}

silicon::ast::Node *silicon::ast::Null::create(compiler::Context *ctx, ast::Type *llvm_type) {
    if (!llvm_type) llvm_type = ctx->type(nullptr);

    auto *node = new Null(llvm_type);

    node->loc = parse_location(ctx->loc);

    return node;
}

llvm::Value *silicon::ast::Null::codegen(compiler::Context *ctx) {
    llvm::Type *t = llvm_type->codegen(ctx);

    if (!t) t = ctx->expected_type;

    if (!t) fail_codegen("Error: Can't detect suitable type");

    return ctx->llvm_ir_builder.CreateLoad(llvm::ConstantPointerNull::get(t->getPointerTo()));
}

silicon::node_t silicon::ast::Null::type() {
    return node_t::NULL_PTR;
}
