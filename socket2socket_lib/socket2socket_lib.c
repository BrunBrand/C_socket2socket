//
// Created by brunob on 9/26/24.
//

#include "socket2socket_lib.h"
#include "../quick_socket//socket_util.h"
#include "../quick_socket/socket_server.h"


fd_set mask, rmask;
int maxfd;

char buffer[BUFFER_SIZE];
char is_alive;

unsigned char client_socket_is_free[MAX_CONNECTIONS];

struct server_connections server_conns;


struct server_connection *get_next_free_connection() {
    for (int i=0; i<MAX_CONNECTIONS; i++) {
        if (server_conns.connections[i].is_free) {
            return &server_conns.connections[i];
        }
    }
    return NULL;
}


struct server_connection* get_next_nonfree_connection() {
    for (int i=0; i<MAX_CONNECTIONS; i++) {
        if (!server_conns.connections[i].is_free) {
            return &server_conns.connections[i];
        }
    }
    return NULL;
}

void check_for_new_client(const int socket_server, const char* remote_host, const char* remote_port, struct server_connection *connection) {
    if (FD_ISSET(socket_server, &rmask)) {

        struct sockaddr_in remote = {0};
        struct addrinfo hints = {0}, *res = NULL;

        socklen_t addrlen = sizeof(remote);

        connection->is_free = 0;

        connection->socket_client =
            accept(
                socket_server,
                (struct sockaddr *) &remote,
                &addrlen
            );

        if (connection->socket_client < 0) exit_with_sys_msg("accept");

        printf("Connection request from %s, port %d\n",
            inet_ntoa(remote.sin_addr),
            ntohs(remote.sin_port)
            );

        if (connection->socket_client > maxfd) maxfd = connection->socket_client;

        if ((connection->socket_remote = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            exit_with_sys_msg("socket");
        }

        if (connection->socket_remote > maxfd) maxfd = connection->socket_remote;

        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        if (getaddrinfo(remote_host, remote_port, &hints, &res) != 0) {
            exit_with_sys_msg("getaddrinfo");
        }

        const struct addrinfo *p = NULL;
        for (p=res; p!= NULL; p=p->ai_next) {
            if (connect(connection->socket_remote, p->ai_addr, p->ai_addrlen) == 0) {
                break;
            }
        }

        if (p==NULL) {
            close(connection->socket_remote);
            exit_with_sys_msg("connect");
        }

        printf("Bidirectional connection established\n");
        printf("(%s:%s) <-> (localhost)\n", remote_host, remote_port);
        FD_SET(connection->socket_remote, &mask);
        FD_SET(connection->socket_client, &mask);


    }
}


void manage_socket_in(struct server_connection *connection) {
    if (connection->is_free != 0) return;

    if (!(FD_ISSET(connection->socket_remote, &rmask))) return;

    printf("remote -> local (connection #%d) ", connection->index);
    fflush(stdout);

    size_t count;
    if ((count = recv(connection->socket_remote, buffer, BUFFER_SIZE, 0)) == -1) {
        perror("read");
        EOC_s2s(connection);
    } else {
        if (count == 0) {
            EOC_s2s(connection);
        } else {
            if ((count = send(connection->socket_client, buffer, count, 0)) == -1) {
                perror("write");
                fflush(stderr);
                EOC_s2s(connection);
            } else {
                printf("%lu Bytes\n", count);
                fflush(stdout);
            }
        }
    }
}

void manage_socket_out(struct server_connection *connection) {
    if (connection->is_free != 0) return;
    if (!(FD_ISSET(connection->socket_client, &rmask))) return;

    printf("local -> remote (connection #%d ) ", connection->index);
    fflush(stdout);

    size_t count;
    if ((count = recv(connection->socket_client, buffer, sizeof(buffer), 0)) == -1) {
        perror("read");
        EOC_s2s(connection);
    } else {
        if (count == 0) {
            EOC_s2s(connection);
        } else {
            if ((count = send(connection->socket_remote, buffer, count, 0)) == -1) {
                perror("write");
                EOC_s2s(connection);
            } else {
                printf("%lu bytes\n", count);
                fflush(stdout);
            }
        }
    }
}

void run_server(const char *remote_host, const char *remote_port, const char *local_port) {

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        server_conns.connections[i].index = i;
    }

    const int socket_server = setup_tcp_server_socket(local_port);

    mark_all_client_sockets_as_free();

    FD_ZERO(&mask);
    FD_SET(socket_server, &mask);

    is_alive = 1;

    maxfd = socket_server;

    while (is_alive) {
        rmask = mask;
        const int nfound = select(maxfd + 1, &rmask, NULL, NULL, &(struct timeval){.tv_sec = 5, .tv_usec = 0});
        if (nfound < 0) {
            if (errno == EINTR) {
                printf("Interrupted by system call\n");
                continue;
            }
            exit_with_sys_msg("select");
        }

        struct server_connection *connection = NULL;

        if ((connection = get_next_free_connection()) == NULL) {
            printf("There are no connections available at this time\n");
        } else {
            if (FD_ISSET(socket_server, &rmask)) {
                check_for_new_client(socket_server, remote_host, remote_port, connection);
            }
        }

        if ((connection = get_next_nonfree_connection()) != NULL) {
            manage_socket_in(connection);
            manage_socket_out(connection);
        }

        fflush(stdout);
        fflush(stderr);
    }
    close(socket_server);
}

void stop_server() {
    is_alive = 0;
}

void EOC_s2s(struct server_connection *connection) {
    close(connection->socket_client);
    close(connection->socket_remote);
    FD_CLR (connection->socket_client, &mask);
    FD_CLR (connection->socket_remote, &mask);
    connection->socket_remote = -1;
    connection->socket_client = -1;
    connection->is_free = 1;
    printf("End of connection\n");
    fflush(stdout);
    fflush(stderr);
}


void mark_all_client_sockets_as_free() {
    for (int index = 0; index < MAX_CONNECTIONS; index++) {
        server_conns.connections[index].is_free = 1;
    }
}
