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

unsigned char client_socket_is_free[MAX_CONNECTIONS];
int client_socket[MAX_CONNECTIONS];
int remote_socket[MAX_CONNECTIONS];

fd_set mask, rmask;
#define BUFFER_SIZE 1500

char buffer[BUFFER_SIZE];

void mark_all_client_sockets_as_free();

void EOC_s2s(int index) {
    close(client_socket[index]);
    close(remote_socket[index]);
    FD_CLR (client_socket[index], &mask);
    FD_CLR (remote_socket[index], &mask);
    client_socket_is_free[index] = 1;
    printf("End of connection\n");
    fflush(stdout);
    fflush(stderr);
}

void exit_with_sys_msg(const char* msg) {
    perror(msg);
    exit(1);
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

int main(const int argc, char **argv) {

    printf("\n");

    if (argc < 4 || argc > 5) {
        printf("Usage: socket2socket remote_host remote_port local_port\n\n");
        exit(1);
    }

    printf("Initialised.\n");

    int server_socket, addrlen;
    int index, count;
    struct sockaddr_in server;
    struct sockaddr_in remote;
    struct addrinfo hints, *res;
    const char *remote_host = argv[1];
    const char *remote_port = argv[2];
    const char *local_port = argv[3];
    struct timeval timeout;

    if ( (server_socket = create_server_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        exit_with_sys_msg("Error opening socket");
    }

    apply_server_config(&server, local_port);

    const int opt = 1;
    // Forcefully attach socket to the port (SO_REUSEADDR)
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        exit_with_sys_msg("setsockopt failed");
    }

    if ((bind(server_socket, (struct sockaddr *) &server, sizeof (server))) < 0) {
        exit_with_sys_msg("bind");
    }

    if (listen(server_socket, SOMAXCONN) < 0) {
        exit_with_sys_msg("listen");
    }

    printf("Listening in port %d\n", atoi(local_port));
    // printf("Press <Control+C> to Quit\n\n");

    mark_all_client_sockets_as_free();

    FD_ZERO(&mask);
    FD_SET(server_socket, &mask);

    int maxfd = server_socket;

    while (1) {
        rmask = mask;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        const int nfound = select(maxfd + 1, &rmask, NULL, NULL, &timeout);
        if (nfound < 0) {
            if (errno == EINTR) {
                printf("Interrupted by system call\n");
                continue;
            }
            exit_with_sys_msg("select");
        }

        if (FD_ISSET(server_socket, &rmask)) {
            addrlen = sizeof(remote);
            for (index = 0; index < MAX_CONNECTIONS; index++)
                if (client_socket_is_free[index])
                    break;
            if (index == MAX_CONNECTIONS) {
                printf("There are no connections available at this time\n");
            } else {
                client_socket_is_free[index] = 0;
                client_socket[index] =
                        accept(server_socket, (struct sockaddr *) &remote, &addrlen);

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
                                perror("\nwrite");
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
    exit(0);
}

void mark_all_client_sockets_as_free() {
    for (int index = 0; index < MAX_CONNECTIONS; index++) {
        client_socket_is_free[index] = 1;
    }
}
