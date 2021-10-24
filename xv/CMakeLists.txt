set(CURSES_NEED_NCURSES ON)
find_package(Curses REQUIRED)

add_executable(xvemu
    "${CMAKE_CURRENT_LIST_DIR}/dev/kb.c"
    "${CMAKE_CURRENT_LIST_DIR}/dev/lpm20.c"
    "${CMAKE_CURRENT_LIST_DIR}/cross_clock_posix.c"
    "${CMAKE_CURRENT_LIST_DIR}/main.c")
target_include_directories(xvemu PRIVATE "${CMAKE_CURRENT_LIST_DIR}" ${CURSES_INCLUDE_DIRS})
target_link_libraries(xvemu ${CURSES_LIBRARIES} vcpu)
