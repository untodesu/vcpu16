cmake_minimum_required(VERSION 3.10)
project(VCPU16 LANGUAGES C VERSION 0.0.1)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

option(VCPU_BUILD_AS "Build VCPU16 assembler (AS)" ON)
option(VCPU_BUILD_DIS "Build VCPU16 disassembler (DIS)" ON)
option(VCPU_BUILD_XV1 "Build XV-1 emulator (VC16 computer)" ON)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Core runtime
add_subdirectory(vcpu)

# Assembler.
if(VCPU_BUILD_AS)
    message("-- Building VCPU assembler")
    add_subdirectory(as)
endif()

# Disassembler
if(VCPU_BUILD_DIS)
    message("-- Building VCPU disassembler")
    add_subdirectory(dis)
endif()

# Full emulator
if(VCPU_BUILD_XV1)
    message("-- Building XV-1 emulator")
    add_subdirectory(xv)
endif()
