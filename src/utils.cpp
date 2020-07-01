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


#include "utils.h"
#include <llvm/IR/Function.h>


silicon::value_pair_t::value_pair_t() = default;

std::string silicon::replace_all(std::string str, const std::string &from, const std::string &to) {
    size_t start_pos = 0;

    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);

        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }

    return str;
}

llvm::Type *silicon::detect_type(llvm::Type *type) {
    if (type->isPointerTy()) type = type->getPointerElementType();

    if (type->isFunctionTy()) type = ((llvm::FunctionType *) type)->getReturnType();

    return type;
}

llvm::Type *silicon::detect_type(llvm::Value *value) {
    llvm::Type *type = value->getType();

    return detect_type(type);
}

bool silicon::compare_types(llvm::Type *type1, llvm::Type *type2) {
    if (type1->isPointerTy()) {
        return type2->isPointerTy()
               && compare_types(type1->getPointerElementType(), type2->getPointerElementType());
    }

    if (type1->isArrayTy()) {
        return type2->isArrayTy()
               && type1->getArrayNumElements() == type2->getArrayNumElements()
               && compare_types(type1->getArrayElementType(), type2->getArrayElementType());
    }

    if (type1->isVoidTy()) return type2->isVoidTy();

    if (type1->isIntegerTy()) return type2->isIntegerTy(type1->getIntegerBitWidth());

    if (type1->isHalfTy()) return type2->isHalfTy();

    if (type1->isFloatTy()) return type2->isFloatTy();

    if (type1->isDoubleTy()) return type2->isDoubleTy();

    return false;
}

bool silicon::compare_types(llvm::Value *value1, llvm::Value *value2) {
    llvm::Type *type1 = detect_type(value1);
    llvm::Type *type2 = detect_type(value2);

    return compare_types(type1, type2);
}

std::string silicon::parse_type(llvm::Type *type) {
    if (type->isVoidTy()) return "void";

    if (type->isIntegerTy()) {
        unsigned bits = type->getIntegerBitWidth();

        if (bits == 1) return "bool";

        return "i" + std::to_string(bits);
    }

    if (type->isHalfTy()) return "f16";

    if (type->isFloatTy()) return "f32";

    if (type->isDoubleTy()) return "f64";

    if (type->isPointerTy() && type->getPointerElementType()->isIntegerTy(8))
        return "string";

    return "unknown";
}

std::string silicon::parse_location(yy::location location) {
    std::string l = (location.begin.filename ? location.begin.filename->c_str() : "(undefined)");

    l += ":" + std::to_string(location.begin.line);

    l += ":" + std::to_string(location.begin.column) + "-" + std::to_string(location.end.column);

    return l;
}
