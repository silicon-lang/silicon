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


#include "silicon/CodeGen/CGContinue.h"


using namespace llvm;
using namespace silicon::codegen;
using namespace silicon::parser::AST;


CGContinue::CGContinue(Continue *node) : Node{node}, Continue{node} {
}

Value *CGContinue::codegen(Context *ctx) {
    loop_points_t *loop_points = ctx->loop_points;

    if (!loop_points) fail("Error: Unexpected \"continue\" outside loop");

    return ctx->llvm_ir_builder.CreateBr(loop_points->continue_point);
}
