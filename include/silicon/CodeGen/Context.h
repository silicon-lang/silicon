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


#ifndef SILICON_CONTEXT_H
#define SILICON_CONTEXT_H


#include <map>
#include <memory>
#include <string>
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"


namespace silicon::codegen {

    struct loop_points_t {
        loop_points_t() = default;

        llvm::BasicBlock *break_point = nullptr;
        llvm::BasicBlock *continue_point = nullptr;
    };

    class CGNode;

    class CGCodeBlock;

    class CGInterface;

    class Context {
    protected:
        CGCodeBlock *code_block = nullptr;

    public:
        llvm::LLVMContext llvm_ctx;
        llvm::IRBuilder<> llvm_ir_builder;
        std::unique_ptr<llvm::Module> llvm_module;
        std::unique_ptr<llvm::legacy::FunctionPassManager> llvm_fpm;

        std::map<std::string, llvm::Type *> types;

        std::map<std::string, CGInterface *> interfaces;

        llvm::Type *expected_type = nullptr;

        loop_points_t *loop_points = nullptr;

        explicit Context(const std::string &library_name);

        virtual ~Context() = default;

        /* ------------------------- Blocks ------------------------- */

        void operator++();

        void operator--();

        llvm::ReturnInst *def_return(llvm::Value *value = nullptr);

        /* ------------------------- Interfaces ------------------------- */

        CGInterface *def_interface(const std::string &name, CGInterface *interface);

        CGInterface *interface(const std::string &name);

        /* ------------------------- Types ------------------------- */

        llvm::Type *def_type(const std::string &name, llvm::Type *type);

        llvm::Type *type(const std::string &name);

        llvm::Type *void_type();

        llvm::Type *bool_type();

        llvm::Type *int_type(unsigned int bits);

        llvm::Type *float_type(unsigned int bits);

        llvm::Type *string_type();

        bool is_interface(llvm::Type *type);

        bool compare_types(llvm::Value *value1, llvm::Value *value2);

        bool compare_types(llvm::Type *type1, llvm::Type *type2);

        llvm::Value *cast_type(CGNode *value, llvm::Type *type);

        llvm::Value *cast_type(llvm::Value *value, llvm::Type *type);

        std::string stringify_type(llvm::Type *type);

        /* ------------------------- Literals ------------------------- */

        llvm::Value *bool_lit(bool value);

        llvm::Value *number_lit(const std::string &value);

        llvm::Value *int_lit(long long int value, unsigned int bits);

        llvm::Value *float_lit(const std::string &value, unsigned int bits);

        llvm::Value *float_lit(const std::string &value, llvm::Type *type);

        /* ------------------------- Memory ------------------------- */

        llvm::AllocaInst *get_alloca(const std::string &name);

        llvm::Value *alloc(const std::string &name, llvm::Type *type);

        llvm::StoreInst *store(llvm::Value *value, llvm::Value *ptr);

        llvm::LoadInst *load(llvm::Value *ptr, const std::string &name = "");

        friend class CGCodeBlock;
    };

}


#endif //SILICON_CONTEXT_H
