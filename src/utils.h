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


#ifndef SILICON_UTILS_H
#define SILICON_UTILS_H


#include <string>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <parser/location.hh>


namespace silicon {

    struct value_pair_t {
        value_pair_t();

        llvm::Value *left = nullptr;
        llvm::Value *right = nullptr;
    };

    enum class node_t {
        NODE,

        BLOCK,

        NULL_PTR,
        BOOLEAN_LIT,
        NUMBER_LIT,
        STRING_LIT,

        VARIABLE_DEFINITION,
        VARIABLE,

        PROTOTYPE,
        FUNCTION,
        RETURN,
        FUNCTION_CALL,

        BINARY_OP,
        UNARY_OP,

        IF,
    };

    enum class binary_operation_t {
        ASSIGN,
        STAR,
        SLASH,
        PERCENT,
        PLUS,
        MINUS,
        CARET,
        AND,
        OR,
        DOUBLE_LESSER,
        DOUBLE_BIGGER,
        TRIPLE_BIGGER,
        LESSER,
        LESSER_EQUAL,
        EQUAL,
        NOT_EQUAL,
        BIGGER_EQUAL,
        BIGGER,
//        STAR_STAR,
//        AND_AND,
//        OR_OR,
    };

    enum class unary_operation_t {
        PLUS_PLUS,
        MINUS_MINUS,
        MINUS,
        NOT,
//        PLUS,
//        TILDE,
//        AND,
    };

    std::string replace_all(std::string str, const std::string &from, const std::string &to);

    llvm::Type *detect_type(llvm::Type *type);

    llvm::Type *detect_type(llvm::Value *value);

    bool compare_types(llvm::Value *value1, llvm::Value *value2);

    std::string parse_location(yy::location location);

}


#endif //SILICON_UTILS_H
