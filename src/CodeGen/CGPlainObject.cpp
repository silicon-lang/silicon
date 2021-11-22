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


#include "silicon/CodeGen/CGPlainObject.h"
#include "silicon/CodeGen/CGInterface.h"


using namespace std;
using namespace llvm;
using namespace silicon::codegen;
using namespace silicon::parser::AST;


CGPlainObject::CGPlainObject(PlainObject *node) : Node{node}, PlainObject{node} {
}

Value *CGPlainObject::codegen(Context *ctx) {
    llvm::Type *type = ctx->expected_type;

    if (!type) fail("TypeError: Can't detect suitable type");

    string type_name = ctx->stringify_type(type);

    if (!ctx->is_interface(type)) fail("TypeError: Can't cast object to type <" + type_name + ">");

    CGInterface *interface = ctx->interface(type_name);

    AllocaInst *var = ctx->llvm_ir_builder.CreateAlloca(type);

    unsigned bits = type->getScalarSizeInBits();

    if (bits > 0 && bits % 8 == 0) var->setAlignment(bits / 8);

    map<string, Node *>::iterator it;

    llvm::Type *expected_type = ctx->expected_type;

    for (const auto &property: interface->get_properties(ctx)) {
        string name = property.first;
        bool exists = false;

        for (it = properties.begin(); it != properties.end(); it++) {
            string prop = it->first;

            if (name == prop) {
                exists = true;

                break;
            }
        }

        if (!exists) fail("Error: Property <" + name + "> is missing from interface <" + type_name + ">");
    }

    for (it = properties.begin(); it != properties.end(); it++) {
        string name = it->first;

        uint64_t index = interface->property_index(ctx, name);

        if (index == -1)
            fail("Error: Interface <" + type_name + "> has no property named <" + name + ">");

        ctx->expected_type = type->getStructElementType(index);

        ctx->store(dynamic_cast<CGNode *>(it->second)->codegen(ctx), ctx->llvm_ir_builder.CreateStructGEP(var, index));
    }

    ctx->expected_type = expected_type;

    return ctx->load(var);
}
