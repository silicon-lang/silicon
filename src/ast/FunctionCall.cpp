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
#include "compiler/Context.h"
#include "FunctionCall.h"


silicon::ast::FunctionCall::FunctionCall(std::string callee, std::vector<Node *> args) : callee(std::move(callee)),
                                                                                         args(std::move(args)) {
}

silicon::ast::Node *
silicon::ast::FunctionCall::create(compiler::Context *ctx, std::string callee, std::vector<Node *> args) {
    auto *node = new FunctionCall(std::move(callee), std::move(args));

    node->loc = parse_location(ctx->loc);

    return node;
}

llvm::Value *silicon::ast::FunctionCall::codegen(compiler::Context *ctx) {
    llvm::Function *calleeFunc = ctx->llvm_module->getFunction(callee);

    if (!calleeFunc) fail_codegen("Undefined function <" + callee + ">");

    llvm::FunctionType *calleeType = calleeFunc->getFunctionType();
    std::vector<llvm::Value *> argsV;

    llvm::Type *expected_type = ctx->expected_type;

    for (unsigned i = 0, e = args.size(); i != e; ++i) {
        ctx->expected_type = calleeType->getFunctionParamType(i);

        Node *arg = args[i];

        argsV.push_back(arg->codegen(ctx));

        ctx->expected_type = expected_type;

        if (!argsV.back())
            return nullptr;
    }

    return ctx->llvm_ir_builder.CreateCall(calleeFunc, argsV);
}

silicon::node_t silicon::ast::FunctionCall::type() {
    return node_t::FUNCTION_CALL;
}
