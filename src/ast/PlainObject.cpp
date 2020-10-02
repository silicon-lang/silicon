//
//   Copyright 10/2/20 Ardalan Amini
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


#include "PlainObject.h"
#include "compiler/Context.h"


silicon::ast::PlainObject::PlainObject(std::map<std::string, Node *> value) : value(MOVE(value)) {
}

silicon::ast::Node *
silicon::ast::PlainObject::create(silicon::compiler::Context *ctx, std::map<std::string, Node *> value) {
    auto *node = new PlainObject(MOVE(value));

    node->loc = parse_location(ctx->loc);

    return node;
}

llvm::Value *silicon::ast::PlainObject::codegen(compiler::Context *ctx) {
    llvm::Type *type = ctx->expected_type;

    if (!type) fail_codegen("TypeError: Can't detect suitable type");

    std::string type_name = parse_type(type);

    if (!is_interface(type)) fail_codegen("TypeError: Can't cast object to type <" + type_name + ">");

    Interface *interface = ctx->interface(type_name);

    llvm::AllocaInst *var = ctx->llvm_ir_builder.CreateAlloca(type);

    unsigned bits = type->getScalarSizeInBits();

    if (bits > 0 && bits % 8 == 0) var->setAlignment(bits / 8);

    std::map<std::string, Node *>::iterator it;

    llvm::Type *expected_type = ctx->expected_type;

    for (it = value.begin(); it != value.end(); it++) {
        std::string name = it->first;

        long index = interface->property_index(name);

        if (index == -1)
            fail_codegen("Error: Interface <" + type_name + "> has no property named <" + name + ">");

        ctx->expected_type = type->getStructElementType(index);

        ctx->store(it->second->codegen(ctx), ctx->llvm_ir_builder.CreateStructGEP(var, index));
//
//        ctx->llvm_ir_builder.CreateStructGEP(var, index);
//
//        ctx->def_op(binary_operation_t::ASSIGN, ctx->var(it->first, interface), it->second)->codegen(ctx);
//        std::cout << it->first  // string (key)
//                  << ':'
//                  << it->second   // string's value
//                  << std::endl;
    }

    ctx->expected_type = expected_type;

    return ctx->load(var);
}

silicon::node_t silicon::ast::PlainObject::type() {
    return node_t::PLAIN_OBJECT;
}
