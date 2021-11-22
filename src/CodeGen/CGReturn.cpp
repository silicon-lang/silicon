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


#include "silicon/CodeGen/CGReturn.h"


using namespace llvm;
using namespace silicon::codegen;
using namespace silicon::parser::AST;


CGReturn::CGReturn(Return *node) : Node{node}, Return{node} {
}

Value *CGReturn::codegen(Context *ctx) {
    Value *ret;

    if (!value) {
        ret = ctx->def_return();

        if (!ret)
            fail(
                    "TypeError: Expected function to return <"
                    + ctx->stringify_type(ctx->expected_type)
                    + ">, got <void> instead."
            );

        return ret;
    }

    Value *v = dynamic_cast<CGNode *>(value)->codegen(ctx);

    ret = ctx->def_return(v);

    if (!ret)
        fail(
                "TypeError: Expected function to return <"
                + ctx->stringify_type(ctx->expected_type)
                + ">, got <"
                + ctx->stringify_type(v->getType())
                + "> instead."
        );

    return ret;
}
