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


#include "Cast.h"
#include "compiler/Context.h"


silicon::ast::Cast::Cast(Node *value, llvm::Type *llvm_type) : value(value), llvm_type(llvm_type) {
}

silicon::ast::Node *
silicon::ast::Cast::create(compiler::Context *ctx, Node *value, llvm::Type *llvm_type) {
    auto *node = new Cast(value, llvm_type);

    node->loc = parse_location(ctx->loc);

    return node;
}

llvm::Value *silicon::ast::Cast::codegen(compiler::Context *ctx) {
    llvm::Value *v;

    if (value->type(node_t::NUMBER_LIT)
        && (llvm_type->isIntegerTy()
            || llvm_type->isFloatingPointTy())) {
        llvm::Type *expected_type = ctx->expected_type;

        ctx->expected_type = llvm_type;

        v = value->codegen(ctx);

        ctx->expected_type = expected_type;

        return v;
    }

    v = value->codegen(ctx);

    llvm::Type *t = v->getType();

    if (compare_types(t, llvm_type)) return v;

    if (!llvm::CastInst::isCastable(t, llvm_type)) {
        fail_codegen(
                "Error: Unsupported cast: <"
                + parse_type(t)
                + "> as <"
                + parse_type(llvm_type)
                + ">"
        );
    }

    return ctx->llvm_ir_builder.CreateCast(
            llvm::CastInst::getCastOpcode(
                    v,
                    true,
                    llvm_type,
                    true
            ),
            v,
            llvm_type,
            "cast"
    );
}

silicon::node_t silicon::ast::Cast::type() {
    return silicon::node_t::CAST;
}
