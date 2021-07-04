cmake_minimum_required(VERSION 3.10)
project(v16dasm LANGUAGES C VERSION 0.0.1)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(v16dasm "${CMAKE_CURRENT_LIST_DIR}/dasm.c")
target_link_libraries(v16dasm v16)
