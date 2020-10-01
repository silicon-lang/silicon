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


#include "NumberLiteral.h"
#include "compiler/Context.h"


silicon::ast::NumberLiteral::NumberLiteral(std::string value) {
    value = replace_all(value, "_", "");

    this->value = value;
}

silicon::ast::Node *silicon::ast::NumberLiteral::create(compiler::Context *ctx, std::string value) {
    auto *node = new NumberLiteral(MOVE(value));

    node->loc = parse_location(ctx->loc);

    return node;
}

llvm::Value *silicon::ast::NumberLiteral::codegen(compiler::Context *ctx) {
    llvm::Type *llvm_type = ctx->expected_type;

    if (!llvm_type
        || (
                llvm_type->isIntegerTy()
                && llvm_type->getIntegerBitWidth() == 1
        )) {
        if (std::string::npos == value.find('.'))
            return ctx->llvm_ir_builder.getInt32(stoi(value));

        return llvm::ConstantFP::get(ctx->float_type(64), value);
    }

    if (llvm_type->isIntegerTy()) return ctx->llvm_ir_builder.getIntN(llvm_type->getIntegerBitWidth(), stoll(value));

    if (llvm_type->isFloatingPointTy()) return llvm::ConstantFP::get(llvm_type, value);

    if (std::string::npos == value.find('.'))
        return ctx->llvm_ir_builder.getInt32(stoi(value));

    return llvm::ConstantFP::get(ctx->float_type(64), value);
}

silicon::node_t silicon::ast::NumberLiteral::type() {
    return node_t::NUMBER_LIT;
}
