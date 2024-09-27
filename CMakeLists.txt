cmake_minimum_required(VERSION 3.17)
project(socket2socket)

find_package(Catch2 3 REQUIRED)

set(CMAKE_C_STANDARD 11)

add_subdirectory(quick_socket)
add_subdirectory(socket2socket_lib)
add_subdirectory(test)

add_executable(socket2socket main.c)

target_link_libraries(socket2socket PRIVATE socket2socket_lib quick_socket)

