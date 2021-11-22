cmake_minimum_required(VERSION 3.16)

project(silicon)

set(CMAKE_CXX_STANDARD 20)

#------------------------- LLVM -------------------------

SET(LLVM_INCLUDE_DOCS OFF)
SET(LLVM_INCLUDE_BENCHMARKS OFF)
SET(LLVM_ENABLE_BINDINGS OFF)
set(LLVM_ENABLE_PROJECTS "lld" CACHE STRING "Enable LLVM projects" FORCE)

add_subdirectory(llvm-project/llvm)

get_directory_property(LLVM_VERSION DIRECTORY llvm-project/llvm DEFINITION LLVM_VERSION)

message(STATUS "Using LLVM ${LLVM_VERSION}")

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
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/silicon-parser/include
        ${PROJECT_SOURCE_DIR}/src
        llvm-project/llvm/include
        llvm-project/lld/include
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_BINARY_DIR}/llvm-project/llvm/include"
)

add_executable(silicon
        src/main.cpp
        src/CodeGen/codegen.cpp
        src/CodeGen/Context.cpp
        src/CodeGen/CGNode.cpp
        src/CodeGen/CGType.cpp
        src/CodeGen/CGBinaryOperation.cpp
        src/CodeGen/CGUnaryOperation.cpp
        src/CodeGen/CGBooleanLiteral.cpp
        src/CodeGen/CGNumberLiteral.cpp
        src/CodeGen/CGString.cpp
        src/CodeGen/CGPlainObject.cpp
        src/CodeGen/CGPrototype.cpp
        src/CodeGen/CGFunction.cpp
        src/CodeGen/CGFunctionCall.cpp
        src/CodeGen/CGBreak.cpp
        src/CodeGen/CGContinue.cpp
        src/CodeGen/CGReturn.cpp
        src/CodeGen/CGCodeBlock.cpp
        src/CodeGen/CGVariable.cpp
        src/CodeGen/CGVariableDefinition.cpp
        src/CodeGen/CGInterface.cpp
        src/CodeGen/CGFor.cpp
        src/CodeGen/CGWhile.cpp
        src/CodeGen/CGLoop.cpp
        src/CodeGen/CGIf.cpp
        )

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
        ${PROJECT_SOURCE_DIR}/silicon-parser/lib/libSiliconParser.a
        ${llvm_libs}
        lldCore
        lldCommon
        lldCOFF
        lldDriver
        lldELF
        lldMinGW
        lldWasm
        )
