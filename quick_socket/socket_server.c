//
// Created by brunob on 9/26/24.
//

#include "socket_server.h"



int setup_tcp_server_socket(const char *port) {
    struct addrinfo hints = {0};

    hints.ai_family 	= AF_UNSPEC;
    hints.ai_flags 		= AI_PASSIVE;
    hints.ai_socktype 	= SOCK_STREAM;
    hints.ai_protocol 	= IPPROTO_TCP;

    struct addrinfo *server_addrinfo;

    int value = 0;
    if ((value = getaddrinfo(NULL, port, &hints, &server_addrinfo)) != 0) {
        exit_with_user_msg("getaddrinfo() failed", gai_strerror(value));
    }

    int socket_server = -1;

    for (const struct addrinfo *addr = server_addrinfo; addr != NULL; addr = addr->ai_next) {
        socket_server = socket(server_addrinfo->ai_family, server_addrinfo->ai_socktype, server_addrinfo->ai_protocol);
        if(socket_server < 0) continue;

        int opt = 1;

        if (setsockopt(socket_server, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
            perror("setsockopt(SO_REUSEPORT) failed");
            exit(EXIT_FAILURE);
        }
        if((bind(socket_server, server_addrinfo->ai_addr, server_addrinfo->ai_addrlen) < 0)){
            perror("bind");
            socket_server = -1;
            close(socket_server);
            continue;
        }

        if ((listen(socket_server, SOMAXCONN) < 0)) {
            perror("listen");
            socket_server = -1;
            close(socket_server);
            continue;
        }

        struct sockaddr_storage localAddr;
        socklen_t addrSize = sizeof(localAddr);
        if(getsockname(socket_server, (struct sockaddr*) &localAddr, &addrSize) < 0)
            exit_with_sys_msg("getsockname() failed");

        fputs("Listening to ", stdout);
        print_socket_address((struct sockaddr*) &localAddr, stdout);
        fputc('\n', stdout);
        break;
    }

    freeaddrinfo(server_addrinfo);
    return socket_server;
}

int accept_tcp_conn(const int socket_server) {
    struct sockaddr_storage client;

    socklen_t client_len = sizeof(client);

    const int socket_client = accept(socket_server, (struct sockaddr *) &client, &client_len);
    if(socket_client < 0) exit_with_sys_msg("accept() failed");

    fputs("Handling client ", stdout);
    print_socket_address((struct sockaddr *) &client, stdout);
    fputc('\n', stdout);

    return socket_client;
}

void recv_block(const int socket_client) {
    char buffer[BUFSIZ];

    ssize_t bytes_received = recv(socket_client, buffer, BUFSIZ, 0);

    if(bytes_received < 0)
        exit_with_sys_msg("recv() failed");

    while(bytes_received > 0){
        const ssize_t bytes_sent = send(socket_client, buffer, bytes_received, 0);
        if(bytes_sent < 0)
            exit_with_sys_msg("send() failed");
        else if (bytes_sent != bytes_received)
            exit_with_user_msg("send()", "sent unexpected number of bytes");

        bytes_received = recv(socket_client, buffer, BUFSIZ, 0);
        if(bytes_received < 0){
            exit_with_sys_msg("recv() failed");
        }
    }

    close(socket_client);
}
