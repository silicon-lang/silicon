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


#include "silicon/CodeGen/CGString.h"


using namespace std;
using namespace llvm;
using namespace silicon::codegen;
using namespace silicon::parser::AST;


CGString::CGString(String *node) : Node{node}, String{node} {
}

Value *CGString::codegen(Context *ctx) {
    string name = "string." + value;

    GlobalVariable *gs = ctx->llvm_module->getNamedGlobal(name);

    if (!gs) gs = ctx->llvm_ir_builder.CreateGlobalString(value, name);

    return ctx->llvm_ir_builder.CreateConstGEP2_64(gs->getValueType(), gs, 0, 0);
}
