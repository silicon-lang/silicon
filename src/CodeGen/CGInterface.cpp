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


#include "silicon/CodeGen/CGInterface.h"


using namespace std;
using namespace llvm;
using namespace silicon::codegen;
using namespace silicon::parser::AST;


CGInterface::CGInterface(Interface *node) : Node{node}, Interface{node} {
}

Value *CGInterface::codegen(Context *ctx) {
    vector<pair<string, CGType *>> props = get_properties(ctx);

    StructType *type = StructType::create(ctx->llvm_ctx, "interface." + name);

    vector<llvm::Type *> body{};
    body.reserve(props.size());

    for (const auto &property: props) body.push_back(property.second->typegen(ctx));

    type->setBody(body);

    ctx->def_interface(name, this);

    ctx->def_type(name, type);

    return nullptr;
}

uint64_t CGInterface::property_index(Context *ctx, const string &property) {
    vector<pair<string, CGType *>> props = get_properties(ctx);
    long index = 0;

    for (const auto &prop: props) {
        if (prop.first == property) return index;

        index++;
    }

    return -1;
}

vector<pair<string, CGType *>> CGInterface::get_properties(Context *ctx) {
    vector<pair<string, CGType *>> props{};
    map<string, bool> names{};

    for (const string &base: bases) {
        if (base == name) fail("TypeError: Interface <" + name + "> can't extend itself");

        auto *type = ctx->type(base);

        if (!ctx->is_interface(type)) fail("TypeError: Interfaces can only extend other interfaces");

        auto *interface = ctx->interface(base);

        for (const auto &property: interface->get_properties(ctx)) {
            string property_name = property.first;

            if (names.count(property_name) > 0) fail("Property <" + property_name + "> can not be redefined");

            names[property_name] = true;

            props.push_back(property);
        }
    }

    for (const auto &property: properties) {
        string property_name = property.first;

        if (names.count(property_name) > 0) fail("Property <" + property_name + "> can not be redefined");

        names[property_name] = true;

        props.push_back({property_name, dynamic_cast<CGType *>(property.second)});
    }

    return props;
}
