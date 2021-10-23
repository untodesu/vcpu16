add_library(vcpu STATIC "${CMAKE_CURRENT_LIST_DIR}/vcpu16.c")
target_include_directories(vcpu PUBLIC "${CMAKE_CURRENT_LIST_DIR}")
