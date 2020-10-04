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


#include "VariableDefinition.h"
#include "compiler/Context.h"


using namespace std;
using namespace silicon;
using namespace ast;
using namespace compiler;


VariableDefinition::VariableDefinition(const string &location, string name, Type *type) : name(MOVE(name)), llvm_type(type) {
    if (!type) silicon_error("Argument <type> is required");

    this->location = location;
}

llvm::Value *VariableDefinition::codegen(Context *ctx) {
    llvm::Type *t = llvm_type->codegen(ctx);

    if (!t) t = ctx->expected_type;

    return ctx->alloc(name, t);
}

node_t VariableDefinition::type() {
    return node_t::VARIABLE_DEFINITION;
}

string VariableDefinition::getName() {
    return name;
}

llvm::Type *VariableDefinition::getLLVMType(Context *ctx) {
    return llvm_type->codegen(ctx);
}
