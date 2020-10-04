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


#define CREATE_NODE(CLS, ...) new CLS(parse_location(loc), ##__VA_ARGS__)


using namespace std;
using namespace silicon::ast;
using namespace silicon::compiler;


Context::Context(const string &filename) : llvm_ir_builder(llvm_ctx) {
    block = CREATE_NODE(Block);

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

void Context::operator++() {
    block = CREATE_NODE(Block, block);
}

void Context::operator--() {
    block = block->getParent();
}

void Context::statements(const vector<Node *> &nodes) {
    block->setStatements(nodes);
}

llvm::Type *Context::def_type(const string &name, llvm::Type *type) {
    if (types.count(name) > 0) fail_codegen("TypeError: Type <" + name + "> can not be defined again.");

    types.insert({name, type});

    return type;
}

Interface *Context::interface(const string &name) {
    auto interface = interfaces.find(name);

    return interface->second;
}

Type *Context::type(llvm::Type *type) const {
    return CREATE_NODE(Type, [type]() -> llvm::Type * {
        return type;
    });
}

Type *Context::type(const string &name) {
    string location = parse_location(loc);

    return CREATE_NODE(Type, [&, location, name]() -> llvm::Type * {
        if (name.empty()) codegen_error(location, "TypeError: Type <" + name + "> not found.");

        auto type = types.find(name);

        if (type == types.end()) codegen_error(location, "TypeError: Type <" + name + "> not found.");

        return type->second;
    });
}

Node *Context::null(Type *type) const {
    if (!type) type = this->type();

    return CREATE_NODE(Null, type);
}

Node *Context::bool_lit(bool value) const {
    return CREATE_NODE(BooleanLiteral, value);
}

Node *Context::num_lit(string value) const {
    return CREATE_NODE(NumberLiteral, MOVE(value));
}

Node *Context::plain_object(map<string, Node *> value) const {
    return CREATE_NODE(PlainObject, MOVE(value));
}

Node *Context::string_lit(string value) const {
    return CREATE_NODE(StringLiteral, MOVE(value));
}

Node *Context::var(const string &name, Node *context) const {
    return CREATE_NODE(Variable, name, context);
}

Node *Context::def_var(const string &name, Type *type) const {
    if (!type) type = this->type();

    return CREATE_NODE(VariableDefinition, name, type);
}

Interface *Context::def_interface(const string &name, vector<pair<string, Type *>> properties, vector<string> parents) {
    if (interfaces.count(name) > 0) fail_codegen("TypeError: Interface <" + name + "> can not be defined again.");

    auto *interface = CREATE_NODE(Interface, name, MOVE(properties), MOVE(parents));

    interfaces.insert({name, interface});

    return interface;
}

pair<string, Type *> Context::def_arg(const string &name, Type *type) {
    return make_pair(name, type);
}

Prototype *Context::def_proto(const string &name, vector<pair<string, Type *>> args, Type *return_type) const {
    if (!return_type) return_type = this->type();

    return CREATE_NODE(Prototype, name, MOVE(args), return_type);
}

Function *Context::def_func(Prototype *prototype, vector<Node *> body) const {
    return CREATE_NODE(Function, prototype, MOVE(body));
}

Return *Context::def_ret(Node *value) const {
    return CREATE_NODE(Return, value);
}

Node *Context::call_func(string callee, vector<Node *> args) const {
    return CREATE_NODE(FunctionCall, MOVE(callee), MOVE(args));
}

Node *Context::def_op(binary_operation_t op, Node *left, Node *right) const {
    return CREATE_NODE(BinaryOperation, op, left, right);
}

Node *Context::def_op(unary_operation_t op, Node *node, bool suffix) const {
    return CREATE_NODE(UnaryOperation, op, node, suffix);
}

Node *Context::def_cast(Node *node, llvm::Type *llvm_type) const {
    return CREATE_NODE(Cast, node, type(llvm_type));
}

Node *Context::def_cast(Node *node, Type *type) const {
    return CREATE_NODE(Cast, node, type);
}

If *Context::def_if(Node *condition, vector<Node *> then_statements, vector<Node *> else_statements) const {
    return CREATE_NODE(If, condition, MOVE(then_statements), MOVE(else_statements));
}

Break *Context::def_break() const {
    return CREATE_NODE(Break);
}

Continue *Context::def_continue() const {
    return CREATE_NODE(Continue);
}

Loop *Context::def_loop(vector<Node *> body) const {
    return CREATE_NODE(Loop, MOVE(body));
}

While *Context::def_while(Node *condition, vector<Node *> body) const {
    return CREATE_NODE(While, condition, MOVE(body));
}

For *Context::def_for(Node *definition, Node *condition, Node *stepper, vector<Node *> body) const {
    return CREATE_NODE(For, definition, condition, stepper, MOVE(body));
}

/* ------------------------- CODEGEN ------------------------- */

llvm::Type *Context::void_type() {
    return llvm_ir_builder.getVoidTy();
}

llvm::Type *Context::bool_type() {
    return llvm_ir_builder.getInt1Ty();
}

llvm::Type *Context::int_type(unsigned int bits) {
    return llvm_ir_builder.getIntNTy(bits);
}

llvm::Type *Context::float_type(unsigned int bits) {
    switch (bits) {
        case 16:
            return llvm_ir_builder.getHalfTy();
        case 32:
            return llvm_ir_builder.getFloatTy();
        case 64:
            return llvm_ir_builder.getDoubleTy();
        default:
            fail_codegen("TypeError: Float type with <" + to_string(bits) + "> bits does not exist.");
    }
}

llvm::Type *Context::string_type() {
    return llvm_ir_builder.getInt8PtrTy();
}

void Context::fail_codegen(const string &error) const noexcept {
    codegen_error(parse_location(loc), error);
}

llvm::Value *Context::codegen() {
    return block->codegen(this);
}

llvm::AllocaInst *Context::get_alloca(const string &name) {
    return block->get_alloca(name);
}

llvm::Value *Context::alloc(const string &name, llvm::Type *type) {
    return block->alloc(this, name, type);
}

llvm::StoreInst *Context::store(llvm::Value *value, llvm::Value *ptr) {
    return llvm_ir_builder.CreateStore(value, ptr);
}

llvm::LoadInst *Context::load(llvm::Value *ptr, const string &name) {
    return llvm_ir_builder.CreateLoad(ptr, name);
}
