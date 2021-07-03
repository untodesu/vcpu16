cmake_minimum_required(VERSION 3.10)
project(V16EXEC LANGUAGES C VERSION 0.0.1)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(V16EXEC "${CMAKE_CURRENT_LIST_DIR}/exec.c")
target_link_libraries(V16EXEC V16)
