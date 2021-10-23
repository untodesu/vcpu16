include(RequireGetopt)
add_executable(vcpu-as "${CMAKE_CURRENT_LIST_DIR}/as.c")
target_link_libraries(vcpu-as PRIVATE vcpu)
