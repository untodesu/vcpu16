cmake_minimum_required(VERSION 3.10)
project(v16sys LANGUAGES C VERSION 0.0.1)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FindSDL2.cmake)
if(NOT SDL2_FOUND)
    message(FATAL_ERROR "Failed to find SDL2")
endif()

add_executable(v16sys
    "${CMAKE_CURRENT_LIST_DIR}/GT86.c"
    "${CMAKE_CURRENT_LIST_DIR}/main.c")
target_include_directories(v16sys PRIVATE ${SDL2_INCLUDE_DIR})
target_link_libraries(v16sys v16 ${SDL2_LIBRARY})
if(NOT MSVC)
    target_link_libraries(v16sys m)
endif()
