cmake_minimum_required(VERSION 3.17)
project(socket2socket C)

set(CMAKE_C_STANDARD 11)

add_executable(socket2socket main.c)

target_link_libraries(socket2socket ws2_32.lib)
