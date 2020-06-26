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


#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Target/TargetMachine.h>
#include <iostream>
#include <fstream>
#include "compiler/Context.h"
#include "parser/parser.h"


int main(int argc, char **argv) {
    std::string input = argv[1];
    std::string output = argv[2];

    std::ifstream f(input);
    std::string buffer(std::istreambuf_iterator<char>(f), {});

    silicon::compiler::Context ctx(input);

    ctx.cursor = buffer.c_str();
    ctx.loc.begin.filename = &input;
    ctx.loc.end.filename = &input;

    yy::Parser parser(ctx);

    parser.parse();

    ctx.codegen();

    verifyModule(*ctx.llvm_module);

    // TODO: remove this (keeping it for debugging)
//    ctx.llvm_module->print(llvm::outs(), nullptr);

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    ctx.llvm_module->setTargetTriple(TargetTriple);

    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

    if (!Target) {
        llvm::errs() << Error;

        return 1;
    }

    auto CPU = llvm::sys::getHostCPUName();
    // TODO:
    auto Features = "";

    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    auto TheTargetMachine =
            Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

    ctx.llvm_module->setDataLayout(TheTargetMachine->createDataLayout());
    std::error_code EC;
    llvm::raw_fd_ostream dest(output, EC, llvm::sys::fs::F_None);

    if (EC) {
        llvm::errs() << "Could not open file: " << EC.message();

        return 1;
    }

    llvm::legacy::PassManager pass;
    auto FileType = llvm::TargetMachine::CGFT_ObjectFile;

    if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        llvm::errs() << "TheTargetMachine can't emit a file of this type";

        return 1;
    }

    pass.run(*ctx.llvm_module);
    dest.flush();

    std::cout << "Wrote " << output << std::endl;

    return 0;
}
