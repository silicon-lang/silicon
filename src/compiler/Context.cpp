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


#include "Context.h"
#include "llvm/ADT/STLExtras.h"
#include <utility>


silicon::compiler::Context::Context(const std::string &filename) : llvm_ir_builder(llvm_ctx) {
    block = silicon::ast::Block::create(this);

    llvm_module = llvm::make_unique<llvm::Module>(filename, llvm_ctx);
}

void silicon::compiler::Context::operator++() {
    block = ast::Block::create(this, block);
}

void silicon::compiler::Context::operator--() {
    block = block->getParent();
}

void silicon::compiler::Context::statements(const std::vector<ast::Node *> &nodes) {
    block->setStatements(nodes);
}

llvm::Type *silicon::compiler::Context::bool_type() {
    return llvm_ir_builder.getInt1Ty();
}

llvm::Type *silicon::compiler::Context::int_type(unsigned int bits) {
    return llvm_ir_builder.getIntNTy(bits);
}

llvm::Type *silicon::compiler::Context::float_type(unsigned int bits) {
    switch (bits) {
        case 16:
            return llvm_ir_builder.getHalfTy();
        case 32:
            return llvm_ir_builder.getFloatTy();
        case 64:
            return llvm_ir_builder.getDoubleTy();
        default:
            return nullptr;
    }
}

silicon::ast::Node *silicon::compiler::Context::bool_lit(bool value) {
    return ast::BooleanLiteral::create(this, value);
}

silicon::ast::Node *silicon::compiler::Context::num_lit(std::string value) {
    return ast::NumberLiteral::create(this, std::move(value));
}

silicon::ast::Node *silicon::compiler::Context::string_lit(std::string value) {
    return ast::StringLiteral::create(this, std::move(value));
}

silicon::ast::Node *silicon::compiler::Context::var(const std::string &name) {
    return ast::Variable::create(this, name);
}

silicon::ast::Node *silicon::compiler::Context::def_var(const std::string &name, llvm::Type *type) {
    return ast::VariableDefinition::create(this, name, type);
}

std::pair<std::string, llvm::Type *> silicon::compiler::Context::def_arg(const std::string &name, llvm::Type *type) {
    return std::make_pair(name, type);
}

silicon::ast::Prototype *
silicon::compiler::Context::def_proto(const std::string &name, std::vector<std::pair<std::string, llvm::Type *>> args,
                                      llvm::Type *return_type) {
    return ast::Prototype::create(this, name, std::move(args), return_type);
}

silicon::ast::Function *
silicon::compiler::Context::def_func(ast::Prototype *prototype, std::vector<ast::Node *> body) {
    return ast::Function::create(this, prototype, std::move(body));
}

silicon::ast::Node *silicon::compiler::Context::def_ret(ast::Node *value) {
    return ast::Return::create(this, value);
}

silicon::ast::Node *silicon::compiler::Context::call_func(std::string callee, std::vector<ast::Node *> args) {
    return ast::FunctionCall::create(this, std::move(callee), std::move(args));
}

silicon::ast::Node *silicon::compiler::Context::def_op(binary_operation_t op, ast::Node *left, ast::Node *right) {
    return ast::BinaryOperation::create(this, op, left, right);
}

silicon::ast::Node *silicon::compiler::Context::def_op(unary_operation_t op, ast::Node *node, bool suffix) {
    return ast::UnaryOperation::create(this, op, node, suffix);
}

/* ------------------------- CODEGEN ------------------------- */

llvm::Value *silicon::compiler::Context::codegen() {
    return block->codegen(this);
}

llvm::AllocaInst *silicon::compiler::Context::get_alloca(const std::string &name) {
    return block->get_alloca(name);
}

llvm::Value *silicon::compiler::Context::alloc(const std::string &name, llvm::Type *type) {
    return block->alloc(this, name, type);
}

llvm::Value *silicon::compiler::Context::store(const std::string &name, llvm::Value *value) {
    return block->store(this, name, value);
}

llvm::Value *silicon::compiler::Context::load(const std::string &name) {
    return block->load(this, name);
}
