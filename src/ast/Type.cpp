//
//   Copyright 10/1/20 Ardalan Amini
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


#include "Type.h"
#include "compiler/Context.h"


using namespace std;
using namespace silicon;
using namespace ast;
using namespace compiler;


Type::Type(const string &location, function<llvm::Type *(void)> llvm_type) : location(MOVE(location)),
                                                                             llvm_type(MOVE(llvm_type)) {
}

llvm::Type *Type::codegen(Context *ctx) {
    return llvm_type();
}

node_t Type::type() {
    return node_t::TYPE;
}

bool Type::type(node_t t) {
    return type() == t;
}

void Type::fail_codegen(const string &error) noexcept {
    codegen_error(location, error);
}
