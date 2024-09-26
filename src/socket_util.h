//
// Created by brunob on 9/26/24.
//

#ifndef SOCKET_UTIL_H
#define SOCKET_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>

void print_socket_address(const struct sockaddr *addr, FILE *stream);

void exit_with_sys_msg(const char* msg);
void exit_with_user_msg(const char *msg, const char *detail);


#endif //SOCKET_UTIL_H
