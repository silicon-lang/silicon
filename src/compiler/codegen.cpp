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
#include "codegen.h"
#include "Context.h"
#include "parser/parser.h"


using namespace std;
using namespace silicon;


void compiler::codegen(string input, string output, bool emit_llvm) {
    const clock_t begin_time = clock();

    ifstream f(input);
    string buffer(istreambuf_iterator<char>(f), {});

    Context ctx(input);

    ctx.cursor = buffer.c_str();
    ctx.loc.begin.filename = &input;
    ctx.loc.end.filename = &input;

    yy::Parser parser(ctx);

    parser.parse();

    ctx.codegen();

    verifyModule(*ctx.llvm_module);

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    ctx.llvm_module->setTargetTriple(TargetTriple);
    auto TheTriple = llvm::Triple(TargetTriple);

    string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

    if (!Target) {
        llvm::errs() << Error;

        exit(1);
    }

    auto CPU = llvm::sys::getHostCPUName();
    // TODO:
    string FeaturesStr;

    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    auto TheTargetMachine =
            Target->createTargetMachine(TargetTriple, CPU, FeaturesStr, opt, RM);

    ctx.llvm_module->setDataLayout(TheTargetMachine->createDataLayout());

    llvm::legacy::PassManager pass;

    if (emit_llvm) {
        pass.run(*ctx.llvm_module);

        output += ".ll";

        error_code EC;
        llvm::raw_fd_ostream dest(output, EC, llvm::sys::fs::F_None);

        if (EC) {
            llvm::errs() << "Could not open file: " << EC.message();

            exit(1);
        }

        ctx.llvm_module->print(dest, nullptr);
    } else {
        if (TheTriple.getOS() == llvm::Triple::Win32) {
            output += ".obj";
        } else {
            output += ".o";
        }

        auto FileType = llvm::TargetMachine::CGFT_ObjectFile;

        error_code EC;
        llvm::raw_fd_ostream dest(output, EC, llvm::sys::fs::F_None);

        if (EC) {
            llvm::errs() << "Could not open file: " << EC.message();

            exit(1);
        }

        if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
            llvm::errs() << "TheTargetMachine can't emit a file of this type";

            exit(1);
        }

        pass.run(*ctx.llvm_module);
        dest.flush();
    }

    llvm::llvm_shutdown();

    const clock_t end_time = clock();

    cout << "Wrote \""
         << output
         << "\" in "
         << float(end_time - begin_time) / CLOCKS_PER_SEC
         << " second(s)"
         << endl;
}
