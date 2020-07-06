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


#include <utility>
#include "llvm/IR/Verifier.h"
#include "Function.h"
#include "compiler/Context.h"


silicon::ast::Function::Function(Prototype *prototype, std::vector<Node *> body)
        : prototype(prototype), body(std::move(body)) {
}

silicon::ast::Function *
silicon::ast::Function::create(compiler::Context *ctx, Prototype *prototype, std::vector<Node *> body) {
    auto *node = new Function(prototype, std::move(body));

    node->loc = parse_location(ctx->loc);

    return node;
}

llvm::Function *silicon::ast::Function::codegen(compiler::Context *ctx) {
    std::string name = prototype->getName();

    llvm::Function *function = ctx->llvm_module->getFunction(name);

    if (!function) function = prototype->codegen(ctx);

    if (!function) fail_codegen("Unexpected error");

    if (!function->empty()) fail_codegen("Error: Function cannot be redefined.");

    // Create a new basic block to start insertion into.
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(ctx->llvm_ctx, "entry", function);
    ctx->llvm_ir_builder.SetInsertPoint(BB);

    ctx->operator++();

    ctx->statements(body);

    // Record the function arguments in the NamedValues map.
    for (auto &Arg : function->args()) {
        ctx->alloc(Arg.getName(), Arg.getType());
        ctx->store(Arg.getName(), &Arg);
    }

    llvm::Type *return_type = prototype->getReturnType();

    llvm::Type *expected_type = ctx->expected_type;

    ctx->expected_type = return_type;

    auto *result = (llvm::ReturnInst *) ctx->codegen();

    if (!result) {
        llvm::Type *fnReturnT = function->getReturnType();
        ast::Node *node = nullptr; // default: fnReturnT->isVoidTy()

        if (fnReturnT->isIntegerTy(1)) node = ctx->bool_lit(false);
        else if (fnReturnT->isIntegerTy() || fnReturnT->isFloatingPointTy()) node = ctx->num_lit("0");

        result = ctx->def_ret(node)->codegen(ctx);
    }

    // Finish off the function.

    ctx->expected_type = expected_type;

    ctx->operator--();

    if (!return_type
        && result->getReturnValue()
        && !result->getReturnValue()->getType()->isVoidTy()) {
        function->removeFromParent();

        return ctx->def_func(
                        prototype->setReturnType(result->getReturnValue()->getType()),
                        body
                )
                ->codegen(ctx);
    }

    // Validate the generated code, checking for consistency.
    verifyFunction(*function);

    return function;
}

silicon::node_t silicon::ast::Function::type() {
    return node_t::FUNCTION;
}

silicon::ast::Function *silicon::ast::Function::externalLinkage() {
    prototype->externalLinkage();

    return this;
}
