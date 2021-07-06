cmake_minimum_required(VERSION 3.10)
project(v16exec LANGUAGES C VERSION 0.0.1)

include(CommonV16)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(v16exec "${CMAKE_CURRENT_LIST_DIR}/exec.c")
target_enable_warnings(v16exec)
target_link_libraries(v16exec v16)
