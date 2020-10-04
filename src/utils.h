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


using namespace std;


namespace silicon {

    struct value_pair_t {
        value_pair_t() = default;

        llvm::Value *left = nullptr;
        llvm::Value *right = nullptr;
    };

    struct loop_points_t {
        loop_points_t() = default;

        llvm::BasicBlock *break_point = nullptr;
        llvm::BasicBlock *continue_point = nullptr;
    };

    enum class node_t {
        NODE,

        TYPE,

        INTERFACE,

        BLOCK,

        NULL_PTR,
        BOOLEAN_LIT,
        NUMBER_LIT,
        STRING_LIT,
        PLAIN_OBJECT,

        VARIABLE_DEFINITION,
        VARIABLE,

        PROTOTYPE,
        FUNCTION,
        RETURN,
        FUNCTION_CALL,

        CAST,
        BINARY_OP,
        UNARY_OP,

        IF,
        LOOP,
        WHILE,
        FOR,
        BREAK,
        CONTINUE,
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

    string replace_all(string str, const string &from, const string &to);

    bool is_interface(llvm::Type *type);

    bool compare_types(llvm::Type *type1, llvm::Type *type2);

    bool compare_types(llvm::Value *value1, llvm::Value *value2);

    string parse_type(llvm::Type *type);

    string parse_location(yy::location location);

    void codegen_error(const string &location, const string &error) noexcept __attribute__ ((__noreturn__));

    void silicon_error(const string &error) noexcept __attribute__ ((__noreturn__));

}

#define MOVE(V) move(V)


#endif //SILICON_UTILS_H
