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


#include <string>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "ast/Node.h"
#include "ast/Block.h"
#include "ast/Null.h"
#include "ast/BooleanLiteral.h"
#include "ast/NumberLiteral.h"
#include "ast/StringLiteral.h"
#include "ast/VariableDefinition.h"
#include "ast/Variable.h"
#include "ast/Prototype.h"
#include "ast/Function.h"
#include "ast/Return.h"
#include "ast/FunctionCall.h"
#include "ast/BinaryOperation.h"
#include "ast/UnaryOperation.h"
#include "ast/If.h"
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
        llvm::IRBuilder<> llvm_ir_builder;

        llvm::Type *expected_type = nullptr;

        explicit Context(const std::string &filename);

        virtual ~Context() = default;

        void operator++();

        void operator--();

        void statements(const std::vector<ast::Node *> &nodes);

        llvm::Type *void_type();

        llvm::Type *bool_type();

        llvm::Type *int_type(unsigned int bits);

        llvm::Type *float_type(unsigned int bits);

        llvm::Type *string_type();

        ast::Node *null(llvm::Type *type = nullptr);

        ast::Node *bool_lit(bool value);

        ast::Node *num_lit(std::string value);

        ast::Node *string_lit(std::string value);

        ast::Node *var(const std::string &name);

        ast::Node *def_var(const std::string &name, llvm::Type *type = nullptr);

        static std::pair<std::string, llvm::Type *> def_arg(const std::string &name, llvm::Type *type);

        ast::Prototype *def_proto(const std::string &name, std::vector<std::pair<std::string, llvm::Type *>> args,
                                  llvm::Type *return_type = nullptr);

        ast::Function *def_func(ast::Prototype *prototype, std::vector<ast::Node *> body);

        ast::Return *def_ret(ast::Node *value = nullptr);

        ast::Node *call_func(std::string callee, std::vector<ast::Node *> args = {});

        ast::Node *def_op(binary_operation_t op, ast::Node *left, ast::Node *right);

        ast::Node *def_op(unary_operation_t op, ast::Node *node, bool suffix = false);

        ast::If *def_if(ast::Node *condition, std::vector<ast::Node *> then_statements,
                        std::vector<ast::Node *> else_statements = {});

        ast::While *def_while(ast::Node *condition, std::vector<ast::Node *> body);

        ast::For *def_for(ast::Node *definition, ast::Node *condition, ast::Node *stepper, std::vector<ast::Node *> body);

        /* ------------------------- CODEGEN ------------------------- */

        llvm::ReturnInst *codegen();

        llvm::AllocaInst *get_alloca(const std::string &name);

        llvm::Value *alloc(const std::string &name, llvm::Type *type);

        llvm::Value *store(const std::string &name, llvm::Value *value);

        llvm::Value *load(const std::string &name);

        friend class ast::Block;
    };

}


#endif //SILICON_CONTEXT_H
