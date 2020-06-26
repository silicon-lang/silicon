#
# This module is designed to find/handle re2c binary
#
# Requirement:
#  CMake >= 2.8.3 (for new version of find_package_handle_standard_args)
#
# The following variables will be defined for your use:
#   - RE2C_EXECUTABLE    : re2c (binary) location
#   - RE2C_VERSION       : complete version of re2c (x.y.z)
#   - RE2C_MAJOR_VERSION : major version of re2c
#   - RE2C_MINOR_VERSION : minor version of re2c
#   - RE2C_PATCH_VERSION : patch version of re2c
#
# The following macro is provided:
#   RE2C_TARGET
#
#  Prototype:
#   RE2C_TARGET([NAME <target>] [INPUT <input>] [OUTPUT <output>] [OPTIONS <options>] [DEPENDS <list of files>])
#
# Argument details:
#   - NAME <target> (mandatory):               cmake target name to build the lexer
#   - INPUT <input> (mandatory):               the input file to give to re2c
#   - OUTPUT <output> (mandatory):             the output file generated by re2c
#   - OPTIONS <options> (optionnal):           command line options to pass to re2c
#   - DEPENDS <file1> ... <file2> (optionnal): a list of files the lexer depends on
#
# How to use:
#   1) Copy this file in the root of your project source directory
#   2) Then, tell CMake to search this non-standard module in your project directory by adding to your CMakeLists.txt:
#        set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
#   3) Finally call find_package(RE2C) once
#
# Here is a complete sample to build an executable:
#
#   set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
#
#   find_package(RE2C REQUIRED) # Note: name is case sensitive
#
#   RE2C_TARGET(NAME foo INPUT foo.re2c OUPUT foo.c OPTIONS "-b" DEPENDS foo.h)
#
#   add_executable(foo foo.c)
#


#=============================================================================
# Copyright (c) 2015, julp
#
# Distributed under the OSI-approved BSD License
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#=============================================================================

cmake_minimum_required(VERSION 2.8.3)

########## Private ##########
if (NOT DEFINED RE2C_PUBLIC_VAR_NS)
    set(RE2C_PUBLIC_VAR_NS "RE2C")
endif (NOT DEFINED RE2C_PUBLIC_VAR_NS)
if (NOT DEFINED RE2C_PRIVATE_VAR_NS)
    set(RE2C_PRIVATE_VAR_NS "_${RE2C_PUBLIC_VAR_NS}")
endif (NOT DEFINED RE2C_PRIVATE_VAR_NS)

function(re2c_debug _VARNAME)
    if (${RE2C_PUBLIC_VAR_NS}_DEBUG)
        if (DEFINED ${RE2C_PUBLIC_VAR_NS}_${_VARNAME})
            message("${RE2C_PUBLIC_VAR_NS}_${_VARNAME} = ${${RE2C_PUBLIC_VAR_NS}_${_VARNAME}}")
        else (DEFINED ${RE2C_PUBLIC_VAR_NS}_${_VARNAME})
            message("${RE2C_PUBLIC_VAR_NS}_${_VARNAME} = <UNDEFINED>")
        endif (DEFINED ${RE2C_PUBLIC_VAR_NS}_${_VARNAME})
    endif (${RE2C_PUBLIC_VAR_NS}_DEBUG)
endfunction(re2c_debug)

########## Public ##########

find_program(${RE2C_PUBLIC_VAR_NS}_EXECUTABLE re2c)
if (${RE2C_PUBLIC_VAR_NS}_EXECUTABLE)
    execute_process(COMMAND ${${RE2C_PUBLIC_VAR_NS}_EXECUTABLE} --vernum OUTPUT_VARIABLE ${RE2C_PRIVATE_VAR_NS}_RAW_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
    math(EXPR RE2C_MAJOR_VERSION "${${RE2C_PRIVATE_VAR_NS}_RAW_VERSION} / 10000")
    math(EXPR RE2C_MINOR_VERSION "(${${RE2C_PRIVATE_VAR_NS}_RAW_VERSION} - ${${RE2C_PUBLIC_VAR_NS}_MAJOR_VERSION} * 10000) / 100")
    math(EXPR RE2C_PATCH_VERSION "${${RE2C_PRIVATE_VAR_NS}_RAW_VERSION} - ${${RE2C_PUBLIC_VAR_NS}_MAJOR_VERSION} * 10000 - ${${RE2C_PUBLIC_VAR_NS}_MINOR_VERSION} * 100")
    set(RE2C_VERSION "${${RE2C_PUBLIC_VAR_NS}_MAJOR_VERSION}.${${RE2C_PUBLIC_VAR_NS}_MINOR_VERSION}.${${RE2C_PUBLIC_VAR_NS}_PATCH_VERSION}")

    include(CMakeParseArguments)
    macro(RE2C_TARGET)
        cmake_parse_arguments(PARSED_ARGS "" "NAME;INPUT;OUTPUT;OPTIONS" "DEPENDS" ${ARGN})

        if (NOT PARSED_ARGS_OUTPUT)
            message(FATAL_ERROR "RE2C_TARGET expect an output filename")
        endif (NOT PARSED_ARGS_OUTPUT)
        if (NOT PARSED_ARGS_INPUT)
            message(FATAL_ERROR "RE2C_TARGET expect an input filename")
        endif (NOT PARSED_ARGS_INPUT)
        if (NOT PARSED_ARGS_NAME)
            message(FATAL_ERROR "RE2C_TARGET expect a target name")
        endif (NOT PARSED_ARGS_NAME)
        # TODO:
        # - get_filename_component(PARSED_ARGS_INPUT ${PARSED_ARGS_INPUT} ABSOLUTE)
        # - get_filename_component(PARSED_ARGS_OUTPUT ${PARSED_ARGS_OUTPUT} ABSOLUTE)
        # ?
        add_custom_command(
                OUTPUT ${PARSED_ARGS_OUTPUT}
                COMMAND ${${RE2C_PUBLIC_VAR_NS}_EXECUTABLE} ${PARSED_ARGS_OPTIONS} -o ${PARSED_ARGS_OUTPUT} ${PARSED_ARGS_INPUT}
                DEPENDS ${PARSED_ARGS_INPUT} ${PARSED_ARGS_DEPENDS}
                COMMENT "[RE2C][${PARSED_ARGS_NAME}] Building lexer with re2c ${${RE2C_PUBLIC_VAR_NS}_VERSION}"
        )
        add_custom_target(
                ${PARSED_ARGS_NAME}
                SOURCES ${PARSED_ARGS_INPUT}
                DEPENDS ${PARSED_ARGS_OUTPUT}
        )
    endmacro(RE2C_TARGET)

    include(FindPackageHandleStandardArgs)
    if (${RE2C_PUBLIC_VAR_NS}_FIND_REQUIRED AND NOT ${RE2C_PUBLIC_VAR_NS}_FIND_QUIETLY)
        find_package_handle_standard_args(
                ${RE2C_PUBLIC_VAR_NS}
                REQUIRED_VARS ${RE2C_PUBLIC_VAR_NS}_EXECUTABLE
                VERSION_VAR ${RE2C_PUBLIC_VAR_NS}_VERSION
        )
    else (${RE2C_PUBLIC_VAR_NS}_FIND_REQUIRED AND NOT ${RE2C_PUBLIC_VAR_NS}_FIND_QUIETLY)
        find_package_handle_standard_args(${RE2C_PUBLIC_VAR_NS} "re2c not found" ${RE2C_PUBLIC_VAR_NS}_EXECUTABLE)
    endif (${RE2C_PUBLIC_VAR_NS}_FIND_REQUIRED AND NOT ${RE2C_PUBLIC_VAR_NS}_FIND_QUIETLY)
endif (${RE2C_PUBLIC_VAR_NS}_EXECUTABLE)

# IN (args)
re2c_debug("FIND_REQUIRED")
re2c_debug("FIND_QUIETLY")
re2c_debug("FIND_VERSION")
# OUT
# Linking
re2c_debug("EXECUTABLE")
# Version
re2c_debug("MAJOR_VERSION")
re2c_debug("MINOR_VERSION")
re2c_debug("PATCH_VERSION")
re2c_debug("VERSION")
