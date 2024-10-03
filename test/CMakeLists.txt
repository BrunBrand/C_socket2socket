project(socket2socket_tests)

add_executable(
        socket2socket_tests_run
        test_socket2socket.cpp
        test_server.cpp
)

Include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.4.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

target_link_libraries(socket2socket_tests_run
        Catch2::Catch2WithMain
        socket2socket_lib
        quick_socket)
