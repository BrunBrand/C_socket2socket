//
// Created by brunob on 9/26/24.
//

#include "socket2socket_lib.h"
#include "../quick_socket//socket_util.h"


fd_set mask, rmask;

char buffer[BUFFER_SIZE];
char is_alive;

unsigned char client_socket_is_free[MAX_CONNECTIONS];
int client_socket[MAX_CONNECTIONS];
int remote_socket[MAX_CONNECTIONS];


void run_server(const char *remote_host, const char *remote_port, const char *local_port) {

    int socket_server, addrlen;
    struct sockaddr_in server;
    struct sockaddr_in remote;
    struct addrinfo hints, *res;

    if ( (socket_server = create_server_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        exit_with_sys_msg("Error opening socket");
    }

    apply_server_config(&server, local_port);
    int opt = 1;

    if (setsockopt(socket_server, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEPORT) failed");
        exit(EXIT_FAILURE);
    }
    if ((bind(socket_server, (struct sockaddr *) &server, sizeof (server))) < 0) {
        exit_with_sys_msg("bind");
    }

    if (listen(socket_server, SOMAXCONN) < 0) {
        exit_with_sys_msg("listen");
    }

    printf("Listening in port %d\n", atoi(local_port));
    // printf("Press <Control+C> to Quit\n\n");

    mark_all_client_sockets_as_free();

    FD_ZERO(&mask);
    FD_SET(socket_server, &mask);

    int maxfd = socket_server;

    is_alive = 1;
    while (is_alive) {
        int index, count;
        rmask = mask;
        const int nfound = select(maxfd + 1, &rmask, NULL, NULL, &(struct timeval){.tv_sec = 5, .tv_usec = 0});
        if (nfound < 0) {
            if (errno == EINTR) {
                printf("Interrupted by system call\n");
                continue;
            }
            exit_with_sys_msg("select");
        }

        if (FD_ISSET(socket_server, &rmask)) {
            addrlen = sizeof(remote);
            for (index = 0; index < MAX_CONNECTIONS; index++)
                if (client_socket_is_free[index])
                    break;
            if (index == MAX_CONNECTIONS) {
                printf("There are no connections available at this time\n");
            } else {
                client_socket_is_free[index] = 0;
                client_socket[index] =
                        accept(socket_server, (struct sockaddr *) &remote, &addrlen);

                if (client_socket[index] < 0) {
                    exit_with_sys_msg("accept");
                }

                printf("Connection request from %s, port %d\n",
                       inet_ntoa(remote.sin_addr), ntohs(remote.sin_port));

                if (client_socket[index] > maxfd)
                    maxfd = client_socket[index];

                printf("Connecting to remote host (%s:%s)\n", remote_host, remote_port);

                if ((remote_socket[index] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                    exit_with_sys_msg("socket");
                }

                if (remote_socket[index] > maxfd)
                    maxfd = remote_socket[index];

                memset(&hints, 0, sizeof(hints));
                hints.ai_family = AF_UNSPEC;
                hints.ai_socktype = SOCK_STREAM;
                hints.ai_protocol = IPPROTO_TCP;

                if (getaddrinfo(remote_host, remote_port, &hints, &res) != 0) {
                    exit_with_sys_msg("getaddrinfo");
                }

                const struct addrinfo *p = NULL;
                for (p=res; p!= NULL; p=p->ai_next) {
                    if (connect(remote_socket[index], p->ai_addr, p->ai_addrlen) == 0) {
                        break;
                    }
                }

                if (p==NULL) {
                    close(remote_socket[index]);
                    exit_with_sys_msg("connect");
                }

                printf("Bidirectional connection established\n");
                printf("(%s:%s) <-> (localhost)\n", remote_host, remote_port);
                FD_SET(remote_socket[index], &mask);
                FD_SET(client_socket[index], &mask);
            }
        }

        for (index = 0; index < MAX_CONNECTIONS; index++) {
            if (!client_socket_is_free[index]) {
                if (FD_ISSET(remote_socket[index], &rmask)) {
                    printf("remote -> local (connection #%d) ", index);
                    fflush(stdout);

                    if ((count = recv(remote_socket[index], buffer, BUFFER_SIZE, 0)) == -1) {
                        perror("read");
                        EOC_s2s(index);
                    } else {
                        if (count == 0) {
                            EOC_s2s(index);
                        } else {
                            if ((count = send(client_socket[index], buffer, count, 0)) == -1) {
                                perror("write");
                                fflush(stderr);
                                EOC_s2s(index);
                            } else {
                                printf(" %d Bytes\n", count);
                                fflush(stdout);
                            }
                        }
                    }
                }
            }

            if (!client_socket_is_free[index]) {
                if (FD_ISSET(client_socket[index], &rmask)) {
                    printf("local -> remote (connection #%d)", index);
                    fflush(stdout);
                    if ((count = recv(client_socket[index], buffer, sizeof(buffer), 0)) == -1) {
                        perror("read");
                        EOC_s2s(index);
                    } else {
                        if (count == 0) {
                            EOC_s2s(index);
                        } else {
                            if ((count = send(remote_socket[index], buffer, count, 0)) == -1) {
                                perror("write");
                                EOC_s2s(index);
                            } else {
                                printf(" %d bytes\n", count);
                                fflush(stdout);
                            }
                        }
                    }
                }
            }
        }
        fflush(stdout);
        fflush(stderr);
    }
}

void stop_server() {
    is_alive = 0;
}

int create_server_socket(const u_char protocol_family, const enum __socket_type socket_type,  const u_char protocol) {
    printf("Initializing socket...\n");
    return socket(protocol_family, socket_type, protocol);
}

void apply_server_config(struct sockaddr_in* server, const char* local_port) {
    memset(server, 0, sizeof (struct sockaddr_in));
    server->sin_family = AF_INET;
    server->sin_addr.s_addr = INADDR_ANY;
    server->sin_port = htons(atoi(local_port));
}

void EOC_s2s(const int index) {
    close(client_socket[index]);
    close(remote_socket[index]);
    FD_CLR (client_socket[index], &mask);
    FD_CLR (remote_socket[index], &mask);
    client_socket_is_free[index] = 1;
    printf("End of connection\n");
    fflush(stdout);
    fflush(stderr);
}


void mark_all_client_sockets_as_free() {
    for (int index = 0; index < MAX_CONNECTIONS; index++) {
        client_socket_is_free[index] = 1;
    }
}
