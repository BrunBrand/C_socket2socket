
#include <sys/socket.h>
#include "test_server.hpp"

extern "C"{
#include "../quick_socket/socket_server.h"
}


void TestServer::start(){

    server_thread = new std::thread([this] {
        listen_for_messages();
    });

    while (!initialized) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void TestServer::listen_for_messages() {
    reset_messages();

    listener_socket = setup_tcp_server_socket(std::to_string(listening_port).c_str(), 5);

    initialized = true;
    int new_socket;
    fd_set mask;
    server_running = true;
    while (server_running) {
        FD_ZERO(&mask);
        FD_SET(listener_socket, &mask);

        timeval timeout ={0};
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        int nfound = select(listener_socket+1, &mask, nullptr, nullptr, &timeout);
        if (nfound > 0) {
            if ((new_socket = accept(listener_socket, nullptr, nullptr)) < 0) {
                printf("Failed");

                close(listener_socket);
                return;
            }

            int count;

            char buffer[512];
            do {
                count = recv(new_socket, buffer, 512, 0);
                if (count > 0)
                    messages.append(buffer, count - 1);

            } while (count > 0);

            messages_arrived = true;
        }
    }
}

const char *TestServer::get_messages() const {
    int count = 0;

    do {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } while (messages_arrived == false && count++ < 300);
    while (!messages_arrived){}
    return messages.c_str();
}

void TestServer::reset_messages() {
    messages_arrived = false;
}

TestServer::~TestServer() {

    server_running = false;

    if (server_thread != nullptr) {
        server_thread->join();
        close(listener_socket);
        delete server_thread;
    }
}


