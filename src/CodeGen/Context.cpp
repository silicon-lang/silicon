//
//   Copyright 2021 Ardalan Amini
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


#include <regex>
#include "llvm/ADT/STLExtras.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "silicon/CodeGen/Context.h"
#include "silicon/CodeGen/CGNode.h"
#include "silicon/CodeGen/CGCodeBlock.h"
#include "silicon/CodeGen/CGInterface.h"


using namespace std;
using namespace llvm;
using namespace silicon;
using namespace silicon::codegen;


Context::Context(const string &library_name) : llvm_ir_builder(llvm_ctx) {
    llvm_module = llvm::make_unique<Module>(library_name, llvm_ctx);

    llvm_fpm = llvm::make_unique<legacy::FunctionPassManager>(llvm_module.get());

    // Do simple "peephole" optimizations and bit-twiddling optzns.
    llvm_fpm->add(createInstructionCombiningPass());
    // Reassociate expressions.
    llvm_fpm->add(createReassociatePass());
    // Eliminate Common SubExpressions.
    // TODO: this one seems to slow the executable!
//    llvm_fpm->add(createGVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    llvm_fpm->add(createCFGSimplificationPass());

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

/* ------------------------- Blocks ------------------------- */

void Context::operator++() {
    auto *codeBlock = new CGCodeBlock(new parser::AST::CodeBlock("<virtual>"));

    codeBlock->parent = code_block;

    code_block = codeBlock;
}

void Context::operator--() {
    code_block = code_block->parent;
}

ReturnInst *Context::def_return(Value *value) {
    if (!value) {
        if (expected_type && !compare_types(void_type(), expected_type)) return nullptr;

        return llvm_ir_builder.CreateRetVoid();
    }

    if (expected_type && !compare_types(value->getType(), expected_type)) return nullptr;

    return llvm_ir_builder.CreateRet(value);
}

/* ------------------------- Interfaces ------------------------- */

CGInterface *Context::def_interface(const string &name, CGInterface *interface) {
    if (interfaces.count(name) > 0) interface->fail("TypeError: Interface <" + name + "> can not be defined again.");

    interfaces.insert({name, interface});

    return interface;
}

CGInterface *Context::interface(const string &name) {
    auto interface = interfaces.find(name);

    return interface->second;
}

/* ------------------------- Types ------------------------- */

Type *Context::def_type(const string &name, Type *type) {
    // TODO: fix
//    if (types.count(name) > 0) fail_codegen("TypeError: Type <" + name + "> can not be defined again.");

    types.insert({name, type});

    return type;
}

Type *Context::type(const string &name) {
    // TODO: fix
//    if (name.empty()) codegen_error(location, "TypeError: Type <" + name + "> not found.");

    auto type = types.find(name);

    // TODO: fix
//    if (type == types.end()) codegen_error(location, "TypeError: Type <" + name + "> not found.");

    return type->second;
}

Type *Context::void_type() {
    return llvm_ir_builder.getVoidTy();
}

Type *Context::bool_type() {
    return llvm_ir_builder.getInt1Ty();
}

Type *Context::int_type(unsigned int bits) {
    return llvm_ir_builder.getIntNTy(bits);
}

Type *Context::float_type(unsigned int bits) {
    switch (bits) {
        case 16:
            return llvm_ir_builder.getHalfTy();
        case 32:
            return llvm_ir_builder.getFloatTy();
        case 64:
            return llvm_ir_builder.getDoubleTy();
        default:
            return nullptr;
            // TODO: fix
//            fail_codegen("TypeError: Float type with <" + to_string(bits) + "> bits does not exist.");
    }
}

Type *Context::string_type() {
    return llvm_ir_builder.getInt8PtrTy();
}

bool Context::is_interface(Type *type) {
    // TODO: for now
    return type->isStructTy();
}

bool Context::compare_types(Value *value1, Value *value2) {
    return compare_types(value1->getType(), value2->getType());
}

bool Context::compare_types(Type *type1, Type *type2) {
    if (type1->isPointerTy()) {
        return type2->isPointerTy()
               && compare_types(type1->getPointerElementType(), type2->getPointerElementType());
    }

    if (type1->isArrayTy()) {
        return type2->isArrayTy()
               && type1->getArrayNumElements() == type2->getArrayNumElements()
               && compare_types(type1->getArrayElementType(), type2->getArrayElementType());
    }

    if (type1->isStructTy()) {
        if (!type2->isStructTy()) return false;

        // TODO: is this safe?
        // if (type1->getStructName() == type2->getStructName()) return true;

        if (type1->getStructNumElements() != type2->getStructNumElements()) return false;

        for (unsigned i = 0; i < type1->getStructNumElements(); i++) {
            if (!compare_types(type1->getStructElementType(i), type2->getStructElementType(i))) return false;
        }

        return true;
    }

    if (type1->isVoidTy()) return type2->isVoidTy();

    if (type1->isIntegerTy()) return type2->isIntegerTy(type1->getIntegerBitWidth());

    if (type1->isHalfTy()) return type2->isHalfTy(); // TODO: remove?!

    if (type1->isFloatTy()) return type2->isFloatTy();

    if (type1->isDoubleTy()) return type2->isDoubleTy();

    return false;
}

Value *Context::cast_type(CGNode *value, Type *type) {
    if (value->is_node(parser::AST::node_t::NUMBER_LIT)
        && (type->isIntegerTy()
            || type->isFloatingPointTy())) {
        Type *expectedT = expected_type;

        expected_type = type;

        Value *v = value->codegen(this);

        expected_type = expectedT;

        return v;
    }

    return cast_type(value->codegen(this), type);
}

Value *Context::cast_type(Value *value, Type *type) {
    Type *valueT = value->getType();

    if (compare_types(valueT, type)) return value;

    if (type->isIntegerTy(1)) {
        if (valueT->isVoidTy()) return bool_lit(false);

        if (valueT->isArrayTy()) return bool_lit(true);
    }

    if (!CastInst::isCastable(valueT, type)) return nullptr;

    return llvm_ir_builder.CreateCast(
            CastInst::getCastOpcode(
                    value,
                    true, // TODO: handle unsigned too
                    type,
                    true // TODO: handle unsigned too
            ),
            value,
            type,
            "cast"
    );
}

string Context::stringify_type(Type *type) {
    if (type->isStructTy()) {
        regex re(".*\\.(.*)");
        cmatch match;
        regex_match(type->getStructName().str().c_str(), match, re);

        return match[1];
    }

    if (type->isVoidTy()) return "void";

    if (type->isIntegerTy()) {
        unsigned bits = type->getIntegerBitWidth();

        if (bits == 1) return "bool";

        return "i" + to_string(bits);
    }

    if (type->isHalfTy()) return "f16";

    if (type->isFloatTy()) return "f32";

    if (type->isDoubleTy()) return "f64";

    if (type->isPointerTy() && type->getPointerElementType()->isIntegerTy(8))
        return "string";

    return "unknown";
}

/* ------------------------- Literals ------------------------- */

Value *Context::bool_lit(bool value) {
    return llvm_ir_builder.getInt1(value);
}

Value *Context::number_lit(const string &value) {
    if (!expected_type
        || (
                expected_type->isIntegerTy()
                && expected_type->getIntegerBitWidth() == 1
        )) {
        if (string::npos == value.find('.'))
            return int_lit(stoll(value), 32);

        return float_lit(value, 64);
    }

    if (expected_type->isIntegerTy()) return int_lit(stoll(value), expected_type->getIntegerBitWidth());

    if (expected_type->isFloatingPointTy()) return float_lit(value, expected_type);

    if (string::npos == value.find('.'))
        return int_lit(stoll(value), 32);

    return float_lit(value, 64);
}

Value *Context::int_lit(long long int value, unsigned int bits) {
    return llvm_ir_builder.getIntN(bits, value);
}

Value *Context::float_lit(const string &value, unsigned int bits) {
    return float_lit(value, float_type(bits));
}

Value *Context::float_lit(const string &value, Type *type) {
    return ConstantFP::get(type, value);
}

/* ------------------------- Memory ------------------------- */

AllocaInst *Context::get_alloca(const string &name) {
    return code_block->get_alloca(name);
}

Value *Context::alloc(const string &name, Type *type) {
    return code_block->alloc(this, name, type);
}

StoreInst *Context::store(Value *value, Value *ptr) {
    return llvm_ir_builder.CreateStore(value, ptr);
}

LoadInst *Context::load(Value *ptr, const string &name) {
    return llvm_ir_builder.CreateLoad(ptr, name);
}
