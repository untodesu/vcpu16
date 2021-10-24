include(FindSDL2)
if(NOT SDL2_FOUND)
    message(FATAL_ERROR "Failed to find SDL2")
endif()

add_executable(xvemu
    "${CMAKE_CURRENT_LIST_DIR}/gl.c"
    "${CMAKE_CURRENT_LIST_DIR}/keyboard.c"
    "${CMAKE_CURRENT_LIST_DIR}/lpm25.c"
    "${CMAKE_CURRENT_LIST_DIR}/main.c")
target_include_directories(xvemu PRIVATE "${CMAKE_CURRENT_LIST_DIR}/include" ${SDL2_INCLUDE_DIR})
target_link_libraries(xvemu ${SDL2_LIBRARY} vcpu)
