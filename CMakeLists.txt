cmake_minimum_required(VERSION 3.16)

project(silicon)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

#------------------------- LLVM -------------------------

SET(LLVM_INCLUDE_DOCS OFF)
SET(LLVM_INCLUDE_BENCHMARKS OFF)
SET(LLVM_ENABLE_BINDINGS OFF)
set(LLVM_ENABLE_PROJECTS "lld" CACHE STRING "Enable LLVM projects" FORCE)

add_subdirectory(llvm-project/llvm)

get_directory_property(LLVM_VERSION DIRECTORY llvm-project/llvm DEFINITION LLVM_VERSION)

message(STATUS "Using LLVM ${LLVM_VERSION}")

#------------------------- BISON -------------------------

set(SILICON_BINARY_DIR ${PROJECT_BINARY_DIR}/silicon-project)
set(SILICON_PARSER_OUTPUT ${SILICON_BINARY_DIR}/parser)
file(MAKE_DIRECTORY ${SILICON_PARSER_OUTPUT})

set(SILICON_BISON_INPUT ${PROJECT_SOURCE_DIR}/src/parser/parser.y)
set(SILICON_BISON_OUTPUT ${SILICON_PARSER_OUTPUT}/parser.re)
set(SILICON_BISON_HEADER ${SILICON_PARSER_OUTPUT}/parser.h)

FIND_PACKAGE(BISON 3.5 REQUIRED)

BISON_TARGET(SILICON ${SILICON_BISON_INPUT} ${SILICON_BISON_OUTPUT}
        DEFINES_FILE ${SILICON_BISON_HEADER}
        VERBOSE
        )

message(STATUS "Using Bison ${BISON_VERSION}")

#------------------------- RE2C -------------------------

set(SILICON_RE2C_OUTPUT ${SILICON_PARSER_OUTPUT}/parser.cpp)

FIND_PACKAGE(RE2C 1.3 REQUIRED)

message(STATUS "Using RE2C ${RE2C_VERSION}")

RE2C_TARGET(
        NAME SILICON
        INPUT ${BISON_SILICON_OUTPUT_SOURCE}
        OUTPUT ${SILICON_RE2C_OUTPUT}
)

#------------------------- SILICON -------------------------

set(SILICON_VERSION_MAJOR 0)
set(SILICON_VERSION_MINOR 1)
set(SILICON_VERSION_PATCH 0)
set(SILICON_VERSION "${SILICON_VERSION_MAJOR}.${SILICON_VERSION_MINOR}.${SILICON_VERSION_PATCH}")

configure_file(
        "${PROJECT_SOURCE_DIR}/src/config.h.in"
        "${SILICON_BINARY_DIR}/config.h"
)

include_directories(
        src
        llvm-project/llvm/include
        llvm-project/lld/include
        "${SILICON_BINARY_DIR}"
        "${PROJECT_BINARY_DIR}/llvm-project/llvm/include"
)

set(SILICON_SRC
        src/main.cpp
        ${BISON_SILICON_OUTPUT_HEADER}
        ${SILICON_RE2C_OUTPUT}
        src/utils.cpp
        src/compiler/codegen.cpp
        src/compiler/Context.cpp
        src/ast/Node.cpp
        src/ast/Type.cpp
        src/ast/Interface.cpp
        src/ast/Block.cpp
        src/ast/Null.cpp
        src/ast/BooleanLiteral.cpp
        src/ast/NumberLiteral.cpp
        src/ast/PlainObject.cpp
        src/ast/StringLiteral.cpp
        src/ast/VariableDefinition.cpp
        src/ast/Variable.cpp
        src/ast/Prototype.cpp
        src/ast/Function.cpp
        src/ast/Return.cpp
        src/ast/FunctionCall.cpp
        src/ast/BinaryOperation.cpp
        src/ast/UnaryOperation.cpp
        src/ast/If.cpp
        src/ast/Loop.cpp
        src/ast/While.cpp
        src/ast/For.cpp
        src/ast/Break.cpp
        src/ast/Continue.cpp
        src/ast/Cast.cpp
        )

add_executable(silicon ${SILICON_SRC})

llvm_map_components_to_libnames(llvm_libs
        support core irreader passes xcoreinfo nvptxinfo
        aarch64asmparser amdgpuasmparser armasmparser bpfasmparser hexagonasmparser lanaiasmparser mipsasmparser
        msp430asmparser powerpcasmparser riscvasmparser sparcasmparser systemzasmparser webassemblyasmparser
        x86asmparser
        aarch64codegen amdgpucodegen armcodegen bpfcodegen hexagoncodegen lanaicodegen mipscodegen
        msp430codegen nvptxcodegen powerpccodegen riscvcodegen sparccodegen systemzcodegen webassemblycodegen x86codegen
        xcorecodegen
        )

target_link_libraries(silicon
        ${llvm_libs}
        lldCore
        lldCommon
        lldCOFF
        lldDriver
        lldELF
        lldMinGW
        lldWasm
        )
