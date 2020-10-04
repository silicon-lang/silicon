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


using namespace std;
using namespace silicon;
using namespace ast;
using namespace compiler;


Function::Function(const string &location, Prototype *prototype, vector<Node *> body) : prototype(prototype),
                                                                                        body(MOVE(body)) {
    this->location = location;
}

llvm::Function *Function::codegen(Context *ctx) {
    string name = prototype->getName();

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
        auto *alloca = ctx->alloc(Arg.getName(), Arg.getType());

        ctx->store(&Arg, alloca);
    }

    llvm::Type *return_type = prototype->getReturnType(ctx);

    llvm::Type *expected_type = ctx->expected_type;

    ctx->expected_type = return_type;

    auto *result = (llvm::ReturnInst *) ctx->codegen();

    if (!result) {
        llvm::Type *fnReturnT = function->getReturnType();
        Node *node = nullptr; // default: fnReturnT->isVoidTy()

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
                        prototype->setReturnType(ctx, result->getReturnValue()->getType()),
                        body
                )
                ->codegen(ctx);
    }

    // Validate the generated code, checking for consistency.
    verifyFunction(*function);

    ctx->llvm_fpm->run(*function);

    return function;
}

node_t Function::type() {
    return node_t::FUNCTION;
}

Function *Function::externalLinkage() {
    prototype->externalLinkage();

    return this;
}
