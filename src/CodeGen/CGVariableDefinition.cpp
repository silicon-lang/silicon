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


#include "silicon/CodeGen/CGVariableDefinition.h"
#include "silicon/CodeGen/CGType.h"


using namespace llvm;
using namespace silicon::codegen;
using namespace silicon::parser::AST;


CGVariableDefinition::CGVariableDefinition(VariableDefinition *node) : Node{node}, VariableDefinition{node} {
}

Value *CGVariableDefinition::codegen(Context *ctx) {
    llvm::Type *t = dynamic_cast<CGType *>(type)->typegen(ctx);

    if (!t) t = ctx->expected_type;

    return ctx->alloc(name, t);
}

llvm::Type *CGVariableDefinition::get_type(Context *ctx) {
    return dynamic_cast<CGType *>(type)->typegen(ctx);
}
