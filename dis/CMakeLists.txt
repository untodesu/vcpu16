add_executable(vcpu-dis "${CMAKE_CURRENT_LIST_DIR}/dis.c")
target_link_libraries(vcpu-dis PRIVATE vcpu)
