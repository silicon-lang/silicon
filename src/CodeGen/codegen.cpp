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


#include <fstream>
#include <iostream>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/ADT/Triple.h>
#include "silicon/CodeGen/CGNode.h"
#include "silicon/CodeGen/CGType.h"
#include "silicon/CodeGen/CGInterface.h"
#include "silicon/CodeGen/CGCodeBlock.h"
#include "silicon/CodeGen/CGBooleanLiteral.h"
#include "silicon/CodeGen/CGNumberLiteral.h"
#include "silicon/CodeGen/CGString.h"
#include "silicon/CodeGen/CGPlainObject.h"
#include "silicon/CodeGen/CGVariableDefinition.h"
#include "silicon/CodeGen/CGVariable.h"
#include "silicon/CodeGen/CGBinaryOperation.h"
#include "silicon/CodeGen/CGUnaryOperation.h"
#include "silicon/CodeGen/CGLoop.h"
#include "silicon/CodeGen/CGWhile.h"
#include "silicon/CodeGen/CGFor.h"
#include "silicon/CodeGen/CGBreak.h"
#include "silicon/CodeGen/CGContinue.h"
#include "silicon/CodeGen/CGIf.h"
#include "silicon/CodeGen/CGFunction.h"
#include "silicon/CodeGen/CGPrototype.h"
#include "silicon/CodeGen/CGFunctionCall.h"
#include "silicon/CodeGen/CGReturn.h"
#include "silicon/CodeGen/Context.h"
#include "silicon/CodeGen/codegen.h"
#include "silicon/parser/Parser.h"
#include "silicon/parser/AST/Node.h"


using namespace std;
using namespace llvm;
using namespace silicon;
using namespace silicon::parser;
using namespace silicon::parser::AST;
using namespace silicon::codegen;


#define WALK_NODE(NODE, BASE) return new NODE(dynamic_cast<BASE *>(node));

Node *walker(Node *node) {
    switch (node->node_type()) {
        case node_t::TYPE:
            WALK_NODE(CGType, AST::Type)
        case node_t::INTERFACE:
            WALK_NODE(CGInterface, Interface)
        case node_t::CODE_BLOCK:
            WALK_NODE(CGCodeBlock, CodeBlock)
        case node_t::BOOLEAN_LIT:
            WALK_NODE(CGBooleanLiteral, BooleanLiteral)
        case node_t::NUMBER_LIT:
            WALK_NODE(CGNumberLiteral, NumberLiteral)
//        case node_t::CHAR_LIT:
        case node_t::STRING:
            WALK_NODE(CGString, String)
        case node_t::PLAIN_OBJECT:
            WALK_NODE(CGPlainObject, PlainObject)
//        case node_t::ARRAY:
//        case node_t::NULL_PTR:
        case node_t::VARIABLE_DEFINITION:
            WALK_NODE(CGVariableDefinition, VariableDefinition)
        case node_t::VARIABLE:
            WALK_NODE(CGVariable, Variable)
        case node_t::BINARY_OP:
            WALK_NODE(CGBinaryOperation, BinaryOperation)
        case node_t::UNARY_OP:
            WALK_NODE(CGUnaryOperation, UnaryOperation)
        case node_t::LOOP:
            WALK_NODE(CGLoop, Loop)
        case node_t::WHILE:
            WALK_NODE(CGWhile, While)
        case node_t::FOR:
            WALK_NODE(CGFor, For)
        case node_t::BREAK:
            WALK_NODE(CGBreak, Break)
        case node_t::CONTINUE:
            WALK_NODE(CGContinue, Continue)
        case node_t::IF:
            WALK_NODE(CGIf, If)
        case node_t::FUNCTION:
            WALK_NODE(CGFunction, AST::Function)
        case node_t::PROTOTYPE:
            WALK_NODE(CGPrototype, Prototype)
        case node_t::FUNCTION_CALL:
            WALK_NODE(CGFunctionCall, FunctionCall)
        case node_t::RETURN:
            WALK_NODE(CGReturn, Return)
        default:
            node->fail("CompileError: Unsupported AST node!");
    }
}

void codegen::codegen(string input, string output, bool emit_llvm) {
    const clock_t begin_time = clock();

    ifstream f(input);
    string buffer(istreambuf_iterator<char>(f), {});

    Parser parser(input);

    parser.cursor(buffer.c_str());

    auto *libraryNode = parser.parse(walker);

    auto *library = dynamic_cast<CGNode *>(libraryNode);

    codegen::Context ctx(input);

    library->codegen(&ctx);

    verifyModule(*ctx.llvm_module);

    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();

    auto TargetTriple = sys::getProcessTriple();
    ctx.llvm_module->setTargetTriple(TargetTriple);
    auto TheTriple = Triple(TargetTriple);

    string Error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

    if (!Target) {
        errs() << Error;

        exit(1);
    }

    auto CPU = sys::getHostCPUName();
    // TODO:
    string FeaturesStr;

    TargetOptions opt;
    auto RM = Optional<Reloc::Model>();
    auto TheTargetMachine =
            Target->createTargetMachine(TargetTriple, CPU, FeaturesStr, opt, RM);

    ctx.llvm_module->setDataLayout(TheTargetMachine->createDataLayout());

    legacy::PassManager pass;

    if (emit_llvm) {
        pass.run(*ctx.llvm_module);

        output += ".ll";

        error_code EC;
        raw_fd_ostream dest(output, EC, sys::fs::F_None);

        if (EC) {
            errs() << "Could not open file: " << EC.message();

            exit(1);
        }

        ctx.llvm_module->print(dest, nullptr);
    } else {
        if (TheTriple.getOS() == Triple::Win32) {
            output += ".obj";
        } else {
            output += ".o";
        }

        auto FileType = TargetMachine::CGFT_ObjectFile;

        error_code EC;
        raw_fd_ostream dest(output, EC, sys::fs::F_None);

        if (EC) {
            errs() << "Could not open file: " << EC.message();

            exit(1);
        }

        if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
            errs() << "TheTargetMachine can't emit a file of this type";

            exit(1);
        }

        pass.run(*ctx.llvm_module);
        dest.flush();
    }

    llvm_shutdown();

    const clock_t end_time = clock();

    cout << "Created \""
         << output
         << "\" in "
         << double(end_time - begin_time) / CLOCKS_PER_SEC
         << " second(s)"
         << endl;
}
