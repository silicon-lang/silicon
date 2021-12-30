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


#include "silicon/CodeGen/CGFunctionCall.h"


using namespace std;
using namespace llvm;
using namespace silicon::codegen;
using namespace silicon::parser::AST;


CGFunctionCall::CGFunctionCall(FunctionCall *node) : Node{node}, FunctionCall{node} {
}

Value *CGFunctionCall::codegen(Context *ctx) {
    Function *calleeFunc = ctx->llvm_module->getFunction(callee);

    if (!calleeFunc) fail("Error: Undefined function <" + callee + ">");

    llvm::FunctionType *calleeType = calleeFunc->getFunctionType();
    vector<llvm::Value *> argsV;

    llvm::Type *expected_type = ctx->expected_type;

    vector<string> argNames;
    for (auto &arg: calleeFunc->args()) argNames.push_back(arg.getName());

    bool is_variadic = calleeType->isVarArg();
    unsigned expected_args_count = calleeType->getNumParams();
    size_t args_count = args.size();

    if (args_count < expected_args_count || (!is_variadic && args_count > expected_args_count)) {
        fail(
                "Error: Function \""
                + callee
                + "\" expected <"
                + to_string(expected_args_count)
                + "> parameter(s), got <"
                + to_string(args_count)
                + "> parameter(s) instead."
        );
    }

    for (unsigned i = 0; i != args_count; ++i) {
        if (i < expected_args_count) ctx->expected_type = calleeType->getFunctionParamType(i);
        else ctx->expected_type = nullptr; // variadic

        Node *arg = args[i];

        llvm::Value *value = dynamic_cast<CGNode *>(arg)->codegen(ctx);

        if (ctx->expected_type && !ctx->compare_types(value->getType(), ctx->expected_type)) {
            arg->fail(
                    "TypeError: Expected parameter \""
                    + (string) argNames[i]
                    + "\" to be <"
                    + ctx->stringify_type(ctx->expected_type)
                    + ">, got <"
                    + ctx->stringify_type(value->getType())
                    + "> instead."
            );
        }

        argsV.push_back(value);

        ctx->expected_type = expected_type;

        if (!argsV.back())
            return nullptr;
    }

    return ctx->llvm_ir_builder.CreateCall(calleeFunc, argsV);
}
