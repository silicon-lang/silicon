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


#ifndef SILICON_CONTEXT_H
#define SILICON_CONTEXT_H


#include <map>
#include <string>
#include <llvm/IR/LegacyPassManager.h>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "ast/Node.h"
#include "ast/Type.h"
#include "ast/Block.h"
#include "ast/Null.h"
#include "ast/BooleanLiteral.h"
#include "ast/NumberLiteral.h"
#include "ast/PlainObject.h"
#include "ast/StringLiteral.h"
#include "ast/VariableDefinition.h"
#include "ast/Variable.h"
#include "ast/Interface.h"
#include "ast/Prototype.h"
#include "ast/Function.h"
#include "ast/Return.h"
#include "ast/FunctionCall.h"
#include "ast/BinaryOperation.h"
#include "ast/UnaryOperation.h"
#include "ast/Cast.h"
#include "ast/If.h"
#include "ast/Break.h"
#include "ast/Continue.h"
#include "ast/Loop.h"
#include "ast/While.h"
#include "ast/For.h"
#include "parser/parser.h"


using namespace std;


namespace silicon::compiler {

    using namespace ast;

    class Context {
    private:
        Block *block;

    public:
        const char *cursor{};
        yy::location loc;

        llvm::LLVMContext llvm_ctx;
        unique_ptr<llvm::Module> llvm_module;
        unique_ptr<llvm::legacy::FunctionPassManager> llvm_fpm;
        llvm::IRBuilder<> llvm_ir_builder;

        llvm::Type *expected_type = nullptr;

        loop_points_t *loop_points = nullptr;

        map<string, llvm::Type *> types;
        map<string, Interface *> interfaces;

        explicit Context(const string &filename);

        virtual ~Context() = default;

        void operator++();

        void operator--();

        void statements(const vector<Node *> &nodes);

        llvm::Type *def_type(const string &name, llvm::Type *type);

        Interface *interface(const string &name);

        /* ------------------------- AST ------------------------- */

        Type *type(llvm::Type *type = nullptr) const;

        Type *type(const string &name);

        Node *null(Type *type = nullptr) const;

        [[nodiscard]] Node *bool_lit(bool value) const;

        [[nodiscard]] Node *num_lit(string value) const;

        [[nodiscard]] Node *plain_object(map<string, Node *> value) const;

        [[nodiscard]] Node *string_lit(string value) const;

        Node *var(const string &name, Node *context = nullptr) const;

        Node *def_var(const string &name, Type *type = nullptr) const;

        Interface *
        def_interface(const string &name, vector<pair<string, Type *>> properties, vector<string> parents = {});

        static pair<string, Type *> def_arg(const string &name, Type *type);

        Prototype *def_proto(const string &name, vector<pair<string, Type *>> args, Type *return_type = nullptr) const;

        Function *def_func(Prototype *prototype, vector<Node *> body) const;

        Return *def_ret(Node *value = nullptr) const;

        [[nodiscard]] Node *call_func(string callee, vector<Node *> args = {}) const;

        Node *def_op(binary_operation_t op, Node *left, Node *right) const;

        Node *def_op(unary_operation_t op, Node *node, bool suffix = false) const;

        Node *def_cast(Node *node, llvm::Type *llvm_type) const;

        Node *def_cast(Node *node, Type *type) const;

        If *def_if(Node *condition, vector<Node *> then_statements, vector<Node *> else_statements = {}) const;

        [[nodiscard]] Break *def_break() const;

        [[nodiscard]] Continue *def_continue() const;

        [[nodiscard]] Loop *def_loop(vector<Node *> body) const;

        While *def_while(Node *condition, vector<Node *> body) const;

        For *def_for(Node *definition, Node *condition, Node *stepper, vector<Node *> body) const;

        /* ------------------------- CODEGEN ------------------------- */

        llvm::Type *void_type();

        llvm::Type *bool_type();

        llvm::Type *int_type(unsigned int bits);

        llvm::Type *float_type(unsigned int bits);

        llvm::Type *string_type();

        void fail_codegen(const string &error) const noexcept __attribute__ ((__noreturn__));

        llvm::Value *codegen();

        llvm::AllocaInst *get_alloca(const string &name);

        llvm::Value *alloc(const string &name, llvm::Type *type);

        llvm::StoreInst *store(llvm::Value *value, llvm::Value *ptr);

        llvm::LoadInst *load(llvm::Value *ptr, const string &name = "");

        friend class Block;
    };

}


#endif //SILICON_CONTEXT_H
