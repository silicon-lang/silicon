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


Interface::Interface(const string &location, string name, vector<pair<string, Type *>> properties,
                     vector<string> parents) : name(MOVE(name)), properties(MOVE(properties)), parents(MOVE(parents)) {
    this->location = location;
}

llvm::Value *Interface::codegen(Context *ctx) {
    vector<pair<string, Type *>> props = get_properties(ctx);

    llvm::StructType *type = llvm::StructType::create(ctx->llvm_ctx, "interface." + name);

    vector<llvm::Type *> body{};
    body.reserve(props.size());

    for (const auto &property: props) body.push_back(property.second->codegen(ctx));

    type->setBody(body);

    ctx->def_type(name, type);

    return nullptr;
}

node_t Interface::type() {
    return node_t::INTERFACE;
}

long Interface::property_index(Context *ctx, const string &property) {
    vector<pair<string, Type *>> props = get_properties(ctx);
    long index = 0;

    for (const auto &prop : props) {
        if (prop.first == property) return index;

        index++;
    }

    return -1;
}

vector<pair<string, Type *>> Interface::get_properties(Context *ctx) {
    vector<pair<string, Type *>> props{};
    map<string, bool> names{};

    for (const string &parent : parents) {
        auto *interface = ctx->interface(parent);

        for (const auto &property: interface->get_properties(ctx)) {
            string property_name = property.first;

            if (names.count(property_name) > 0) fail_codegen("Property <" + property_name + "> can not be redefined");

            names[property_name] = true;

            props.push_back(property);
        }
    }

    for (const auto &property: properties) {
        string property_name = property.first;

        if (names.count(property_name) > 0) fail_codegen("Property <" + property_name + "> can not be redefined");

        names[property_name] = true;

        props.push_back(property);
    }

    return props;
}
