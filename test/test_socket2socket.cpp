//
// Created by brunob on 9/26/24.
//

#include <catch2/catch_all.hpp>
#include <thread>
#include <string>

#include "test_server.hpp"

extern "C"{
#include "../socket2socket_lib//socket2socket_lib.h"
#include "../quick_socket/socket_client.h"

}

using Catch::Matchers::Equals;


class TestSocket2SocketFixture {

    std::thread *server_thread;
public:
    TestSocket2SocketFixture() {
        server_thread = new std::thread([]() {
            const std::string host = "127.0.0.1";
            const std::string port = "2222";
            const std::string local_port = "1111";
            run_server(host.c_str(), port.c_str(), local_port.c_str());
        });
    }

    virtual ~TestSocket2SocketFixture() {
        stop_server();
        server_thread->join();
        delete server_thread;
    }
};

TEST_CASE_METHOD(
    TestSocket2SocketFixture,
    "Send message to port 1111 --> message arrives at server listening on port 2222",
    "[E2E][Send]") {

    TestServer test_server(2222);
    test_server.start();

    SECTION("Send one message --> receive one message", "[E2E][Send]") {
        open_send_close("127.0.0.1", "1111", "1234", 5);
        REQUIRE_THAT(test_server.get_messages(), Equals("1234"));
    }
}

TEST_CASE_METHOD(
    TestSocket2SocketFixture,
    "Send messages to port 1111 --> message arrives at server listening on port 2222",
    "[E2E][Send]") {

    TestServer test_server(2222);
    test_server.start();

    SECTION("Send two messages, two clients --> both messages arrive at server", "[E2E][Send]") {
        open_send_close("127.0.0.1", "1111", "message1 ", 10);
        // ReSharper disable once CppExpressionWithoutSideEffects
        test_server.get_messages();
        test_server.reset_messages();
        open_send_close("127.0.0.1", "1111", "message2", 9);

        REQUIRE_THAT(test_server.get_messages(), Equals("message1 message2"));
    }
}