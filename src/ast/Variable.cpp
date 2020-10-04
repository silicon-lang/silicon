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


#include "Variable.h"
#include "compiler/Context.h"


using namespace std;
using namespace silicon;
using namespace ast;
using namespace compiler;


Variable::Variable(const string &location, string name, Node *context) : name(MOVE(name)), context(context) {
    this->location = location;
}

llvm::Value *Variable::codegen(Context *ctx) {
    if (context) return ctx->load(get_pointer(ctx));

    auto *alloca = ctx->get_alloca(name);

    if (!alloca)
        fail_codegen("Variable <" + name + "> is not allocated yet");

    llvm::LoadInst *load = ctx->load(alloca, name);

    unsigned alignment = alloca->getAlignment();

    if (alignment > 0) load->setAlignment(alignment);

    return load;
}

node_t Variable::type() {
    return node_t::VARIABLE;
}

string Variable::getName() {
    return name;
}

llvm::Type *Variable::getType(Context *ctx) {
    if (context) {
        uint64_t index = element_index(ctx);
        auto *var = (Variable *) context;
        llvm::Type *type = var->getType(ctx);

        return type->getStructElementType(index);
    }

    return ctx->get_alloca(name)->getAllocatedType();
}

llvm::Value *Variable::get_pointer(Context *ctx) {
    if (!context) {
        auto *alloca = ctx->get_alloca(name);

        if (!alloca)
            fail_codegen("Variable <" + name + "> is not allocated yet");

        return alloca;
    }

    uint64_t index = element_index(ctx);
    auto *var = (Variable *) context;

    return ctx->llvm_ir_builder.CreateStructGEP(var->get_pointer(ctx), index);
}

uint64_t Variable::element_index(Context *ctx) {
    if (!context->type(node_t::VARIABLE))
        fail_codegen("Can not access property <" + name + "> of non variable");

    auto *var = (Variable *) context;

    llvm::Type *type = var->getType(ctx);
    string type_name = parse_type(type);

    if (!is_interface(type))
        fail_codegen("Can not access property <" + name + "> of <" + type_name + ">");

    Interface *interface = ctx->interface(type_name);

    if (!interface)
        fail_codegen("Can not find interface <" + type_name + ">");

    long index = interface->property_index(ctx, name);

    if (index == -1)
        fail_codegen("Interface <" + type_name + "> has no property named <" + name + ">");

    return index;
}
