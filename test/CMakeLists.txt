project(socket2socket_tests)

add_executable(
        socket2socket_tests_run
        test_socket2socket.cpp
        test_server.cpp
)

target_link_libraries(socket2socket_tests_run
        Catch2::Catch2WithMain
        socket2socket_lib
        quick_socket)
