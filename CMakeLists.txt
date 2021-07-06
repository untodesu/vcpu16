cmake_minimum_required(VERSION 3.10)
project(v16 LANGUAGES C VERSION 0.0.1)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(CommonV16)

add_library(v16 STATIC
    "${CMAKE_CURRENT_LIST_DIR}/V16.c"
    "${CMAKE_CURRENT_LIST_DIR}/V16.h")
target_enable_warnings(v16)
target_include_directories(v16 PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

option(BUILD_ASM "Build assembler" ON)
if(BUILD_ASM)
    add_subdirectory(asm)
endif()

option(BUILD_DASM "Build disassembler" ON)
if(BUILD_DASM)
    add_subdirectory(dasm)
endif()

option(BUILD_EXEC "Build v16exec" ON)
if(BUILD_EXEC)
    add_subdirectory(exec)
endif()

option(BUILD_SYS "Build v16sys" ON)
if(BUILD_SYS)
    add_subdirectory(sys)
endif()
