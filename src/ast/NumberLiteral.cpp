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


using namespace std;
using namespace silicon;
using namespace ast;
using namespace compiler;


NumberLiteral::NumberLiteral(const string &location, string value) {
    this->location = location;

    value = replace_all(value, "_", "");

    this->value = value;
}

llvm::Value *NumberLiteral::codegen(Context *ctx) {
    llvm::Type *llvm_type = ctx->expected_type;

    if (!llvm_type
        || (
                llvm_type->isIntegerTy()
                && llvm_type->getIntegerBitWidth() == 1
        )) {
        if (string::npos == value.find('.'))
            return ctx->llvm_ir_builder.getInt32(stoi(value));

        return llvm::ConstantFP::get(ctx->float_type(64), value);
    }

    if (llvm_type->isIntegerTy()) return ctx->llvm_ir_builder.getIntN(llvm_type->getIntegerBitWidth(), stoll(value));

    if (llvm_type->isFloatingPointTy()) return llvm::ConstantFP::get(llvm_type, value);

    if (string::npos == value.find('.'))
        return ctx->llvm_ir_builder.getInt32(stoi(value));

    return llvm::ConstantFP::get(ctx->float_type(64), value);
}

node_t NumberLiteral::type() {
    return node_t::NUMBER_LIT;
}
