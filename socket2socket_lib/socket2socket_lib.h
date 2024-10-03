//
// Created by brunob on 9/26/24.
//
#ifndef SOCKET2SOCKET_LIB_H
#define SOCKET2SOCKET_LIB_H

#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>
#include <unistd.h>
#include "../quick_socket/portable_sockets.h"
#include "../quick_socket/socket_util.h"
#include "../quick_socket/socket_server.h"

#define MAX_CONNECTIONS 10
#define BUFFER_SIZE 1500

extern fd_set mask, rmask;

extern char buffer[BUFFER_SIZE];
extern char is_alive;

extern unsigned char client_socket_is_free[MAX_CONNECTIONS];

struct server_connection {
    SOCKET socket_client;
    SOCKET socket_remote;
    char is_free;
};

struct server_connections {
    struct server_connection connections[MAX_CONNECTIONS];
};

void run_server(const char *remote_host, const char *remote_port, const char *local_port);

void stop_server();

void EOC_s2s(struct server_connection* connection);
void mark_all_client_sockets_as_free();

#endif //SOCKET2SOCKET_LIB_H
