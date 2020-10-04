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


#include "Interface.h"
#include "compiler/Context.h"


using namespace std;
using namespace silicon;
using namespace ast;
using namespace compiler;


Interface::Interface(const string &location, string name, vector<pair<string, Type *>> properties) : name(MOVE(name)),
                                                                                                     properties(
                                                                                                             MOVE(properties)) {
    this->location = location;
}

llvm::Value *Interface::codegen(Context *ctx) {
    llvm::StructType *type = llvm::StructType::create(ctx->llvm_ctx, "interface." + name);
    vector<llvm::Type *> body{};

    for (const auto &property: properties) body.push_back(property.second->codegen(ctx));

    type->setBody(body);

    ctx->def_type(name, type);

    return nullptr;
}

node_t Interface::type() {
    return node_t::INTERFACE;
}

long Interface::property_index(const string &property) {
    long index = 0;

    for (const auto &prop : properties) {
        if (prop.first == property) return index;

        index++;
    }

    return -1;
}
