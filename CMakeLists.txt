cmake_minimum_required(VERSION 3.16)

project(silicon VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

#------------------------- LLVM -------------------------

FIND_PACKAGE(LLVM 9 REQUIRED)

message(STATUS "Using LLVM ${LLVM_PACKAGE_VERSION}")

include_directories(${LLVM_INCLUDE_DIRS})

add_definitions(${LLVM_DEFINITIONS})

link_directories(${LLVM_LIBRARY_DIRS})

#------------------------- BISON -------------------------

set(SILICON_BISON_INPUT ${PROJECT_SOURCE_DIR}/src/parser/parser.y)
set(SILICON_BISON_OUTPUT ${PROJECT_SOURCE_DIR}/src/parser/parser.re)
set(SILICON_BISON_HEADER ${PROJECT_SOURCE_DIR}/src/parser/parser.h)

FIND_PACKAGE(BISON 3.5 REQUIRED)

BISON_TARGET(silicon_bison ${SILICON_BISON_INPUT} ${SILICON_BISON_OUTPUT}
        DEFINES_FILE ${SILICON_BISON_HEADER})

message(STATUS "Using Bison ${BISON_VERSION}")

#------------------------- RE2C -------------------------

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
set(SILICON_RE2C_OUTPUT ${PROJECT_SOURCE_DIR}/src/parser/parser.cpp)

FIND_PACKAGE(RE2C 1.3 REQUIRED)

message(STATUS "Using RE2C ${RE2C_VERSION}")

RE2C_TARGET(
        NAME silicon_re2c
        INPUT ${BISON_silicon_bison_OUTPUT_SOURCE}
        OUTPUT ${SILICON_RE2C_OUTPUT}
)

#------------------------- SILICON -------------------------

set(SILICON_VERSION_MAJOR 0)
set(SILICON_VERSION_MINOR 1)
set(SILICON_VERSION_PATCH 0)
set(SILICON_VERSION "${SILICON_VERSION_MAJOR}.${SILICON_VERSION_MINOR}.${SILICON_VERSION_PATCH}")

configure_file(
        "${PROJECT_SOURCE_DIR}/src/config.h.in"
        "${PROJECT_BINARY_DIR}/config.h"
)

include_directories(
        src
        "${PROJECT_BINARY_DIR}"
)

add_executable(silicon
        src/main.cpp
        ${SILICON_RE2C_OUTPUT}
        ${BISON_silicon_bison_OUTPUT_HEADER}
        src/utils/CLI11.hpp
        src/utils.cpp src/utils.h
        src/compiler/codegen.cpp src/compiler/codegen.h
        src/compiler/Context.cpp src/compiler/Context.h
        src/ast/Node.cpp src/ast/Node.h
        src/ast/Type.cpp src/ast/Type.h
        src/ast/Interface.cpp src/ast/Interface.h
        src/ast/Block.cpp src/ast/Block.h
        src/ast/Null.cpp src/ast/Null.h
        src/ast/BooleanLiteral.cpp src/ast/BooleanLiteral.h
        src/ast/NumberLiteral.cpp src/ast/NumberLiteral.h
        src/ast/StringLiteral.cpp src/ast/StringLiteral.h
        src/ast/VariableDefinition.cpp src/ast/VariableDefinition.h
        src/ast/Variable.cpp src/ast/Variable.h
        src/ast/Prototype.cpp src/ast/Prototype.h
        src/ast/Function.cpp src/ast/Function.h
        src/ast/Return.cpp src/ast/Return.h
        src/ast/FunctionCall.cpp src/ast/FunctionCall.h
        src/ast/BinaryOperation.cpp src/ast/BinaryOperation.h
        src/ast/UnaryOperation.cpp src/ast/UnaryOperation.h
        src/ast/If.cpp src/ast/If.h
        src/ast/Loop.cpp src/ast/Loop.h
        src/ast/While.cpp src/ast/While.h
        src/ast/For.cpp src/ast/For.h
        src/ast/Break.cpp src/ast/Break.h
        src/ast/Continue.cpp src/ast/Continue.h
        src/ast/Cast.cpp src/ast/Cast.h
        )

llvm_map_components_to_libnames(llvm_libs
        support core irreader passes xcoreinfo nvptxinfo
        aarch64asmparser amdgpuasmparser armasmparser bpfasmparser hexagonasmparser lanaiasmparser mipsasmparser
        msp430asmparser powerpcasmparser riscvasmparser sparcasmparser systemzasmparser webassemblyasmparser
        x86asmparser avrasmparser
        aarch64codegen amdgpucodegen armcodegen avrcodegen bpfcodegen hexagoncodegen lanaicodegen mipscodegen
        msp430codegen nvptxcodegen powerpccodegen riscvcodegen sparccodegen systemzcodegen webassemblycodegen x86codegen
        xcorecodegen
        )

target_link_libraries(silicon -static ${llvm_libs})
