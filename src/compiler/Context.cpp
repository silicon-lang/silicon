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


#include <utility>
#include <llvm/ADT/STLExtras.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include "Context.h"


silicon::compiler::Context::Context(const std::string &filename) : llvm_ir_builder(llvm_ctx) {
    block = silicon::ast::Block::create(this);

    llvm_module = llvm::make_unique<llvm::Module>(filename, llvm_ctx);

    llvm_fpm = llvm::make_unique<llvm::legacy::FunctionPassManager>(llvm_module.get());

    // Do simple "peephole" optimizations and bit-twiddling optzns.
    llvm_fpm->add(llvm::createInstructionCombiningPass());
    // Reassociate expressions.
    llvm_fpm->add(llvm::createReassociatePass());
    // Eliminate Common SubExpressions.
    llvm_fpm->add(llvm::createGVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    llvm_fpm->add(llvm::createCFGSimplificationPass());

    llvm_fpm->doInitialization();

    // Types

    def_type("void", void_type());

    def_type("bool", bool_type());

    def_type("string", string_type());

    def_type("i8", int_type(8));
    def_type("i16", int_type(16));
    def_type("i32", int_type(32));
    def_type("i64", int_type(64));
    def_type("i128", int_type(128));

    def_type("f16", float_type(16));
    def_type("f32", float_type(32));
    def_type("f64", float_type(64));
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

llvm::Type *silicon::compiler::Context::def_type(const std::string &name, llvm::Type *type) {
    if (types.count(name) > 0) fail_codegen("TypeError: Type <" + name + "> can not be defined again.");

    types.insert({name, type});

    return type;
}

llvm::Type *silicon::compiler::Context::void_type() {
    return llvm_ir_builder.getVoidTy();
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
            fail_codegen("TypeError: Float type with <" + std::to_string(bits) + "> bits does not exist.");
    }
}

llvm::Type *silicon::compiler::Context::string_type() {
    return llvm_ir_builder.getInt8PtrTy();
}

silicon::ast::Type *silicon::compiler::Context::type(llvm::Type *type) {
    return ast::Type::create(this, type);
}

silicon::ast::Type *silicon::compiler::Context::type(const std::string &name) {
    return ast::Type::create(this, name);
}

silicon::ast::Node *silicon::compiler::Context::null(ast::Type *type) {
    return ast::Null::create(this, type);
}

silicon::ast::Node *silicon::compiler::Context::bool_lit(bool value) {
    return ast::BooleanLiteral::create(this, value);
}

silicon::ast::Node *silicon::compiler::Context::num_lit(std::string value) {
    return ast::NumberLiteral::create(this, MOVE(value));
}

silicon::ast::Node *silicon::compiler::Context::plain_object(std::map<std::string, ast::Node *> value) {
    return ast::PlainObject::create(this, MOVE(value));
}

silicon::ast::Node *silicon::compiler::Context::string_lit(std::string value) {
    return ast::StringLiteral::create(this, MOVE(value));
}

silicon::ast::Node *silicon::compiler::Context::var(const std::string &name, ast::Node *context) {
    return ast::Variable::create(this, name, context);
}

silicon::ast::Node *silicon::compiler::Context::def_var(const std::string &name, ast::Type *type) {
    return ast::VariableDefinition::create(this, name, type);
}

silicon::ast::Interface *silicon::compiler::Context::interface(const std::string &name) {
    auto interface = interfaces.find(name);

    return interface->second;
}

silicon::ast::Interface *silicon::compiler::Context::def_interface(const std::string &name,
                                                                   std::vector<std::pair<std::string, ast::Type *>> properties) {
    if (interfaces.count(name) > 0) fail_codegen("TypeError: Interface <" + name + "> can not be defined again.");

    auto *interface = ast::Interface::create(this, name, MOVE(properties));

    interfaces.insert({name, interface});

    return interface;
}

std::pair<std::string, silicon::ast::Type *>
silicon::compiler::Context::def_arg(const std::string &name, ast::Type *type) {
    return std::make_pair(name, type);
}

silicon::ast::Prototype *
silicon::compiler::Context::def_proto(const std::string &name, std::vector<std::pair<std::string, ast::Type *>> args,
                                      ast::Type *return_type) {
    return ast::Prototype::create(this, name, MOVE(args), return_type);
}

silicon::ast::Function *
silicon::compiler::Context::def_func(ast::Prototype *prototype, std::vector<ast::Node *> body) {
    return ast::Function::create(this, prototype, MOVE(body));
}

silicon::ast::Return *silicon::compiler::Context::def_ret(ast::Node *value) {
    return ast::Return::create(this, value);
}

silicon::ast::Node *silicon::compiler::Context::call_func(std::string callee, std::vector<ast::Node *> args) {
    return ast::FunctionCall::create(this, MOVE(callee), MOVE(args));
}

silicon::ast::Node *silicon::compiler::Context::def_op(binary_operation_t op, ast::Node *left, ast::Node *right) {
    return ast::BinaryOperation::create(this, op, left, right);
}

silicon::ast::Node *silicon::compiler::Context::def_op(unary_operation_t op, ast::Node *node, bool suffix) {
    return ast::UnaryOperation::create(this, op, node, suffix);
}

silicon::ast::Node *silicon::compiler::Context::def_cast(ast::Node *node, llvm::Type *llvm_type) {
    return ast::Cast::create(this, node, type(llvm_type));
}

silicon::ast::Node *silicon::compiler::Context::def_cast(ast::Node *node, ast::Type *type) {
    return ast::Cast::create(this, node, type);
}

silicon::ast::If *
silicon::compiler::Context::def_if(silicon::ast::Node *condition, std::vector<ast::Node *> then_statements,
                                   std::vector<ast::Node *> else_statements) {
    return ast::If::create(this, condition, MOVE(then_statements), MOVE(else_statements));
}

silicon::ast::Break *silicon::compiler::Context::def_break() {
    return ast::Break::create(this);
}

silicon::ast::Continue *silicon::compiler::Context::def_continue() {
    return ast::Continue::create(this);
}

silicon::ast::Loop *silicon::compiler::Context::def_loop(std::vector<ast::Node *> body) {
    return ast::Loop::create(this, MOVE(body));
}

silicon::ast::While *
silicon::compiler::Context::def_while(ast::Node *condition, std::vector<ast::Node *> body) {
    return ast::While::create(this, condition, MOVE(body));
}

silicon::ast::For *
silicon::compiler::Context::def_for(ast::Node *definition, ast::Node *condition, ast::Node *stepper,
                                    std::vector<ast::Node *> body) {
    return ast::For::create(this, definition, condition, stepper, MOVE(body));
}

/* ------------------------- CODEGEN ------------------------- */

void silicon::compiler::Context::fail_codegen(const std::string &error) const noexcept {
    codegen_error(parse_location(loc), error);
}

llvm::Value *silicon::compiler::Context::codegen() {
    return block->codegen(this);
}

llvm::AllocaInst *silicon::compiler::Context::get_alloca(const std::string &name) {
    return block->get_alloca(name);
}

llvm::Value *silicon::compiler::Context::alloc(const std::string &name, llvm::Type *type) {
    return block->alloc(this, name, type);
}

llvm::StoreInst *silicon::compiler::Context::store(llvm::Value *value, llvm::Value *ptr) {
    return llvm_ir_builder.CreateStore(value, ptr);
}

llvm::LoadInst *silicon::compiler::Context::load(llvm::Value *ptr, const std::string &name) {
    return llvm_ir_builder.CreateLoad(ptr, name);
}
