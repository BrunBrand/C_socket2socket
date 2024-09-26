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

int setup_tcp_client_socket(const char *host, const char *port);



#endif //SOCKET_CLIENT_H
