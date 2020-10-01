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


silicon::ast::Type::Type(std::function<llvm::Type *()> llvm_type) : llvm_type(MOVE(llvm_type)) {
}

silicon::ast::Type *silicon::ast::Type::create(silicon::compiler::Context *ctx, llvm::Type *type) {
    auto *node = new Type([type]() -> llvm::Type * {
        return type;
    });

    node->loc = parse_location(ctx->loc);

    return node;
}

silicon::ast::Type *silicon::ast::Type::create(silicon::compiler::Context *ctx, const std::string &name) {
    std::string location = parse_location(ctx->loc);

    auto *node = new Type([ctx, location, name]() -> llvm::Type * {
        if (name.empty()) codegen_error(location, "TypeError: Type <" + name + "> not found.");

        auto type = ctx->types.find(name);

        if (type == ctx->types.end()) codegen_error(location, "TypeError: Type <" + name + "> not found.");

        return type->second;
    });

    node->loc = location;

    return node;
}

llvm::Type *silicon::ast::Type::codegen(compiler::Context *ctx) {
    return llvm_type();
}

silicon::node_t silicon::ast::Type::type() {
    return node_t::TYPE;
}

bool silicon::ast::Type::type(silicon::node_t t) {
    return type() == t;
}

void silicon::ast::Type::fail_codegen(const std::string &error) noexcept {
    codegen_error(loc, error);
}
