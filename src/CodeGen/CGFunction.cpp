//
//   Copyright 2021 Ardalan Amini
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


#include "llvm/IR/Verifier.h"
#include "silicon/CodeGen/CGFunction.h"
#include "silicon/CodeGen/CGPrototype.h"


using namespace std;
using namespace llvm;
using namespace silicon::codegen;
using namespace silicon::parser::AST;


CGFunction::CGFunction(Function *node) : Node{node}, Function{node} {
}

Value *CGFunction::codegen(Context *ctx) {
    auto *proto = dynamic_cast<CGPrototype *>(prototype);

    string name = proto->name;

    llvm::Function *function = ctx->llvm_module->getFunction(name);

    if (!function) function = (llvm::Function *) proto->codegen(ctx);

    if (!function) fail("Error: Unexpected error!");

    if (!function->empty()) fail("Error: Function cannot be redefined.");

    // Create a new basic block to start insertion into.
    BasicBlock *BB = BasicBlock::Create(ctx->llvm_ctx, "entry", function);
    ctx->llvm_ir_builder.SetInsertPoint(BB);

    ctx->operator++();

    // Record the function arguments in the NamedValues map.
    for (auto &Arg: function->args()) {
        auto *alloca = ctx->alloc(Arg.getName(), Arg.getType());

        ctx->store(&Arg, alloca);
    }

    Type *return_type = proto->get_return_type(ctx);

    Type *expected_type = ctx->expected_type;

    ctx->expected_type = return_type;

    auto *result = (ReturnInst *) dynamic_cast<CGNode *>(body)->codegen(ctx);

    if (!result) {
        Type *fnReturnT = function->getReturnType();
        Value *node = nullptr; // default: fnReturnT->isVoidTy()

        if (fnReturnT->isIntegerTy(1)) node = ctx->bool_lit(false);
        else if (fnReturnT->isIntegerTy() || fnReturnT->isFloatingPointTy()) node = ctx->number_lit("0");

        ctx->def_return(node);
//        result = ctx->def_return(node);
    }

    // Finish off the function.

    ctx->expected_type = expected_type;

    ctx->operator--();

    // TODO: optional return type
//    if (!return_type
//        && result->getReturnValue()
//        && !result->getReturnValue()->getType()->isVoidTy()) {
//        function->removeFromParent();
//
//        return ctx->def_func(
//                        prototype->setReturnType(ctx, result->getReturnValue()->getType()),
//                        body
//                )
//                ->codegen(ctx);
//    }

    // Validate the generated code, checking for consistency.
    verifyFunction(*function);

    ctx->llvm_fpm->run(*function);

    return function;
}
