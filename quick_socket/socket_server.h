//
// Created by brunob on 9/26/24.
//

#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "socket_util.h"
#include "portable_sockets.h"

// For server setup and configuration
SOCKET setup_tcp_server_socket(const char *port);

// For handling clients
SOCKET accept_tcp_conn(SOCKET socket_server);
void recv_block(int socket_client);

// TODO maybe create rcv_nonblock


#endif //SOCKET_SERVER_H
