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
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <asm-generic/errno-base.h>
#include <unistd.h>

#define MAX_CONNECTIONS 10
#define BUFFER_SIZE 1500

extern fd_set mask, rmask;

extern char buffer[BUFFER_SIZE];

extern unsigned char client_socket_is_free[MAX_CONNECTIONS];
extern int client_socket[MAX_CONNECTIONS];
extern int remote_socket[MAX_CONNECTIONS];


void run_server(const char *remote_host, const char *remote_port, const char *local_port);

void apply_server_config(struct sockaddr_in* server, const char* local_port);
int create_server_socket(const u_char protocol_family, const enum __socket_type socket_type, const u_char protocol);

void EOC_s2s(const int index);

void exit_with_sys_msg(const char* msg);
void mark_all_client_sockets_as_free();

#endif //SOCKET2SOCKET_LIB_H
