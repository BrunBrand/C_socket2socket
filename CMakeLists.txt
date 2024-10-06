cmake_minimum_required(VERSION 3.17)
project(socket2socket)
set(CMAKE_C_STANDARD 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

add_subdirectory(quick_socket)
add_subdirectory(socket2socket_lib)
add_subdirectory(test)

add_executable(socket2socket main.c)

include_directories(${GTK4_INCLUDE_DIRS})
link_directories(${GTK4_LIBRARY_DIRS})
add_definitions(${GTK4_CFLAGS_OTHER})

configure_file(${CMAKE_SOURCE_DIR}/UI/builder.ui ${CMAKE_BINARY_DIR}/builder.ui COPYONLY)

target_link_libraries(socket2socket PRIVATE socket2socket_lib quick_socket ${GTK4_LIBRARIES})
