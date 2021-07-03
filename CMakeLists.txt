cmake_minimum_required(VERSION 3.10)
project(V16 LANGUAGES C VERSION 0.0.1)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(V16 STATIC
    "${CMAKE_CURRENT_LIST_DIR}/V16.c"
    "${CMAKE_CURRENT_LIST_DIR}/V16.h")
target_include_directories(V16 PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

option(BUILD_ASM "Build assembler" ON)
if(BUILD_ASM)
    add_subdirectory(asm)
endif()

option(BUILD_DASM "Build disassembler" ON)
if(BUILD_DASM)
    add_subdirectory(dasm)
endif()

option(BUILD_EXEC "Build V16EXEC" ON)
if(BUILD_EXEC)
    add_subdirectory(exec)
endif()
