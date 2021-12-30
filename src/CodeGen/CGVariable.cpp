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


#include "silicon/CodeGen/CGVariable.h"
#include "silicon/CodeGen/CGInterface.h"


using namespace std;
using namespace llvm;
using namespace silicon::codegen;
using namespace silicon::parser::AST;


CGVariable::CGVariable(Variable *node) : Node{node}, Variable{node} {
}

Value *CGVariable::codegen(Context *ctx) {
    if (context) return ctx->load(get_pointer(ctx));

    auto *alloca = ctx->get_alloca(name);

    if (!alloca)
        fail("Variable <" + name + "> is not allocated yet");

    LoadInst *load = ctx->load(alloca, name);

    unsigned alignment = alloca->getAlignment();

    if (alignment > 0) load->setAlignment(alignment);

    return load;
}

llvm::Type *CGVariable::get_type(Context *ctx) {
    if (context) {
        uint64_t index = element_index(ctx);

        llvm::Type *type = dynamic_cast<CGVariable *>(context)->get_type(ctx);

        return type->getStructElementType(index);
    }

    return ctx->get_alloca(name)->getAllocatedType();
}

Value *CGVariable::get_pointer(Context *ctx) {
    if (!context) {
        auto *alloca = ctx->get_alloca(name);

        if (!alloca)
            fail("Variable <" + name + "> is not allocated yet");

        return alloca;
    }

    uint64_t index = element_index(ctx);

    return ctx->llvm_ir_builder.CreateStructGEP(dynamic_cast<CGVariable *>(context)->get_pointer(ctx), index);
}

uint64_t CGVariable::element_index(Context *ctx) {
    if (!context->is_node(node_t::VARIABLE))
        fail("Can not access property <" + name + "> of non variable");

    auto *var = dynamic_cast<CGVariable *>(context);

    llvm::Type *type = var->get_type(ctx);
    string type_name = ctx->stringify_type(type);

    if (!ctx->is_interface(type))
        fail("Can not access property <" + name + "> of <" + type_name + ">");

    CGInterface *interface = ctx->interface(type_name);

    if (!interface)
        fail("Can not find interface <" + type_name + ">");

    uint64_t index = interface->property_index(ctx, name);

    if (index == -1)
        fail("Interface <" + type_name + "> has no property named <" + name + ">");

    return index;
}
