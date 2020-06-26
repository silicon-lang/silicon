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


silicon::ast::Null::Null(llvm::Type *llvm_type) : llvm_type(llvm_type) {
}

silicon::ast::Node *silicon::ast::Null::create(compiler::Context *ctx, llvm::Type *llvm_type) {
    auto *node = new Null(llvm_type);

    node->loc = parse_location(ctx->loc);

    return node;
}

llvm::Value *silicon::ast::Null::codegen(compiler::Context *ctx) {
    if (!llvm_type) llvm_type = ctx->expected_type;

    if (!llvm_type) llvm_type = ctx->llvm_ir_builder.getVoidTy();

    return llvm::ConstantPointerNull::get(llvm_type->getPointerTo());
}

silicon::node_t silicon::ast::Null::type() {
    return silicon::node_t::NULL_PTR;
}
