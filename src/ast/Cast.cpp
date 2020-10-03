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


using namespace silicon;
using namespace ast;
using namespace compiler;


Cast::Cast(Node *value, Type *llvm_type) : value(value), llvm_type(llvm_type) {
    if (!llvm_type) silicon_error("Argument <llvm_type> is required");
}

Node *Cast::create(Context *ctx, Node *value, Type *llvm_type) {
    auto *node = new Cast(value, llvm_type);

    node->loc = parse_location(ctx->loc);

    return node;
}

llvm::Value *Cast::codegen(Context *ctx) {
    llvm::Value *v;
    llvm::Type *llvm_t = llvm_type->codegen(ctx);

    if (value->type(node_t::NUMBER_LIT)
        && (llvm_t->isIntegerTy()
            || llvm_t->isFloatingPointTy())) {
        llvm::Type *expected_type = ctx->expected_type;

        ctx->expected_type = llvm_t;

        v = value->codegen(ctx);

        ctx->expected_type = expected_type;

        return v;
    }

    v = value->codegen(ctx);

    llvm::Type *t = v->getType();

    if (compare_types(t, llvm_t)) return v;

    if (llvm_t->isIntegerTy(1)) {
        if (t->isVoidTy()) return ctx->bool_lit(false)->codegen(ctx);

        if (t->isArrayTy()) return ctx->bool_lit(true)->codegen(ctx);
    }

    if (!llvm::CastInst::isCastable(t, llvm_t)) {
        fail_codegen(
                "Error: Unsupported cast: <"
                + parse_type(t)
                + "> as <"
                + parse_type(llvm_t)
                + ">"
        );
    }

    return ctx->llvm_ir_builder.CreateCast(
            llvm::CastInst::getCastOpcode(
                    v,
                    true,
                    llvm_t,
                    true
            ),
            v,
            llvm_t,
            "cast"
    );
}

node_t Cast::type() {
    return node_t::CAST;
}
