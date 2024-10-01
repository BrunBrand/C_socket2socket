//
// Created by brunob on 9/26/24.
//

#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "socket_util.h"
#include "portable_sockets.h"

SOCKET setup_tcp_client_socket(const char *host, const char *port);

// Opens a socket, sends a message and then closes
void open_send_close(const char *host, const char* port, const char *message, int len);


#endif //SOCKET_CLIENT_H
