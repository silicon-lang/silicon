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


#include <iostream>
#include <string>
#include "config.h"
#include "utils/CLI11.hpp"
#include "silicon/CodeGen/codegen.h"


using namespace std;
using namespace silicon::codegen;


void print_version() {
    cout << "silicon " << SILICON_VERSION << endl;

    exit(0);
}

int main(int argc, char **argv) {
    CLI::App app{"The Silicon Programming Language"};

    app.add_flag_callback(
            "-v,--version",
            print_version,
            "Print version info and exit"
    );

    string input;
    app.add_option(
                    "input",
                    input
            )
            ->type_name("file")
            ->check(CLI::ExistingFile)
            ->required();

    string output = "output";
    app.add_option(
                    "-o,--output",
                    output,
                    "Write output to <filename>",
                    true
            )
            ->type_name("filename");

    bool emit_llvm = false;
    app.add_flag(
            "--emit-llvm",
            emit_llvm,
            "Emit LLVM IR"
    );

    CLI11_PARSE(app, argc, argv);

    codegen(input, output, emit_llvm);

    return 0;
}
