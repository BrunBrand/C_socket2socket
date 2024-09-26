cmake_minimum_required(VERSION 3.17)
project(socket2socket)

set(CMAKE_C_STANDARD 11)

add_subdirectory(src/socket2socket_lib)

add_executable(socket2socket main.c)

target_link_libraries(socket2socket PRIVATE socket2socket_lib)

