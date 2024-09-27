//
// Created by brunob on 9/26/24.
//

#ifndef TEST_SERVER_H
#define TEST_SERVER_H

#include <iostream>
#include <string>
#include <thread>
#include <netinet/in.h>

class TestServer {
private:
    bool initialized;
    bool server_running;
    bool messages_arrived;
    int listening_port;
    std::string messages;
    int listener_socket;
    sockaddr_in server;
    std::thread* server_thread;

public:
    explicit TestServer(const int port):
        initialized(false), server_running(false),
        messages_arrived(false), listening_port(port), server_thread(nullptr){}

    ~TestServer();

    void start();
    const char* get_messages() const;
    void reset_messages();

    void stop();

private:
    void listen_for_messages();

};


#endif //TEST_SERVER_H
