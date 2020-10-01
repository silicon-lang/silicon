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


namespace silicon::compiler {

    class Context {
    private:
        ast::Block *block;

    public:
        const char *cursor{};
        yy::location loc;

        llvm::LLVMContext llvm_ctx;
        std::unique_ptr<llvm::Module> llvm_module;
        std::unique_ptr<llvm::legacy::FunctionPassManager> llvm_fpm;
        llvm::IRBuilder<> llvm_ir_builder;

        llvm::Type *expected_type = nullptr;

        loop_points_t *loop_points = nullptr;

        std::map<std::string, llvm::Type *> types;
        std::map<std::string, ast::Interface *> interfaces;

        explicit Context(const std::string &filename);

        virtual ~Context() = default;

        void operator++();

        void operator--();

        void statements(const std::vector<ast::Node *> &nodes);

        llvm::Type *def_type(const std::string &name, llvm::Type *type);

        llvm::Type *void_type();

        llvm::Type *bool_type();

        llvm::Type *int_type(unsigned int bits);

        llvm::Type *float_type(unsigned int bits);

        llvm::Type *string_type();

        ast::Type *type(llvm::Type *type);

        ast::Type *type(const std::string &name);

        ast::Node *null(ast::Type *type = nullptr);

        ast::Node *bool_lit(bool value);

        ast::Node *num_lit(std::string value);

        ast::Node *string_lit(std::string value);

        ast::Node *var(const std::string &name, ast::Node *context = nullptr);

        ast::Node *def_var(const std::string &name, ast::Type *type = nullptr);

        ast::Interface *interface(const std::string &name);

        ast::Interface *
        def_interface(const std::string &name, std::vector<std::pair<std::string, ast::Type *>> properties);

        static std::pair<std::string, ast::Type *> def_arg(const std::string &name, ast::Type *type);

        ast::Prototype *def_proto(const std::string &name, std::vector<std::pair<std::string, ast::Type *>> args,
                                  ast::Type *return_type = nullptr);

        ast::Function *def_func(ast::Prototype *prototype, std::vector<ast::Node *> body);

        ast::Return *def_ret(ast::Node *value = nullptr);

        ast::Node *call_func(std::string callee, std::vector<ast::Node *> args = {});

        ast::Node *def_op(binary_operation_t op, ast::Node *left, ast::Node *right);

        ast::Node *def_op(unary_operation_t op, ast::Node *node, bool suffix = false);

        ast::Node *def_cast(ast::Node *node, llvm::Type *llvm_type);

        ast::Node *def_cast(ast::Node *node, ast::Type *type);

        ast::If *def_if(ast::Node *condition, std::vector<ast::Node *> then_statements,
                        std::vector<ast::Node *> else_statements = {});

        ast::Break *def_break();

        ast::Continue *def_continue();

        ast::Loop *def_loop(std::vector<ast::Node *> body);

        ast::While *def_while(ast::Node *condition, std::vector<ast::Node *> body);

        ast::For *
        def_for(ast::Node *definition, ast::Node *condition, ast::Node *stepper, std::vector<ast::Node *> body);

        /* ------------------------- CODEGEN ------------------------- */

        void fail_codegen(const std::string &error) const noexcept __attribute__ ((__noreturn__));

        llvm::Value *codegen();

        llvm::AllocaInst *get_alloca(const std::string &name);

        llvm::Value *alloc(const std::string &name, llvm::Type *type);

        llvm::StoreInst *store(llvm::Value *value, llvm::Value *ptr);

        llvm::LoadInst *load(llvm::Value *ptr, const std::string &name = "");

        friend class ast::Block;
    };

}


#endif //SILICON_CONTEXT_H
