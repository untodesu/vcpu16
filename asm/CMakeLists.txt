cmake_minimum_required(VERSION 3.10)
project(V16ASM LANGUAGES C VERSION 0.0.1)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(V16ASM "${CMAKE_CURRENT_LIST_DIR}/asm.c")
target_link_libraries(V16ASM V16)
