#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <winsock.h>
#include <memory.h>

#define MAX_CONNECTIONS 10

unsigned char client_socket_is_free[MAX_CONNECTIONS];
int client_socket[MAX_CONNECTIONS];
int remote_socket[MAX_CONNECTIONS];

fd_set mask, rmask;
#define BUFFER_SIZE 1500

char buffer[BUFFER_SIZE];

void mark_all_client_sockets_as_free();

void
EOC_s2s(int index) {
    closesocket(client_socket[index]);
    closesocket(remote_socket[index]);
    FD_CLR (client_socket[index], &mask);
    FD_CLR (remote_socket[index], &mask);
    client_socket_is_free[index] = 1;
    printf("End of connection\n");
    fflush(stdout);
    fflush(stderr);
}

int
main(int argc, char **argv) {
    WSADATA wsa;

    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }

    printf("Initialised.");

    int request_socket, addrlen;
    int index, count;
    struct sockaddr_in server;
    struct sockaddr_in remote;
    struct hostent *remote_hostent;
    char *remote_host;
    char *remote_port;
    char *local_port;
    int nfound, maxfd;
    struct timeval timeout;

    if ((argc < 4) || (argc > 5)) {
        printf("Usage: socket2socket remote_host remote_port local_port\n\n");
        exit(1);
    }
    remote_host = argv[1];
    remote_port = argv[2];
    local_port = argv[3];

    if ((request_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    memset(&server, 0, sizeof server);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(local_port));
    if ((bind(request_socket, (struct sockaddr *) &server, sizeof server)) < 0) {
        perror("bind");
        exit(1);
    }
    if (listen(request_socket, SOMAXCONN) < 0) {
        perror("listen");
        exit(1);
    }
    printf("Listening in port %d\n", atoi(local_port));
    printf("Press <Control+C> to Quit\n\n");

    mark_all_client_sockets_as_free();
    FD_ZERO(&mask);
    FD_SET(request_socket, &mask);
    maxfd = request_socket;
    while (1) {
        rmask = mask;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        nfound = select(maxfd + 1, &rmask, NULL, NULL, &timeout);
        if (nfound < 0) {
            if (errno == EINTR) {
                printf("Interrupted by system call\n");
                continue;
            }
            perror("select");
            exit(1);
        }

        if (FD_ISSET(request_socket, &rmask)) {
            addrlen = sizeof(remote);
            for (index = 0; index < MAX_CONNECTIONS; index++)
                if (client_socket_is_free[index])
                    break;
            if (index == MAX_CONNECTIONS) {
                printf("There are no connections available at this time\n");
            } else {
                client_socket_is_free[index] = 0;
                client_socket[index] =
                        accept(request_socket, (struct sockaddr *) &remote, &addrlen);
                if (client_socket[index] < 0) {
                    perror("accept");
                    exit(1);
                }
                printf("Connection request from %s, port %d\n",
                       inet_ntoa(remote.sin_addr), ntohs(remote.sin_port));
                if (client_socket[index] > maxfd)
                    maxfd = client_socket[index];
                printf("Connecting to remote host (%s:%s)\n", remote_host, remote_port);
                if ((remote_socket[index] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                    perror("socket");
                    exit(1);
                }
                if (remote_socket[index] > maxfd)
                    maxfd = remote_socket[index];
                if ((remote_hostent = gethostbyname(remote_host)) == 0) {
                    perror("gethostbyname");
                    exit(1);
                }
                memset(&remote, 0, sizeof remote);
                remote.sin_family = AF_INET;
                memcpy(&remote.sin_addr, remote_hostent->h_addr,
                       remote_hostent->h_length);
                remote.sin_port = htons(atoi(remote_port));
                if (connect(remote_socket[index], (struct sockaddr *) &remote, sizeof remote) < 0) {
                    closesocket(remote_socket[index]);
                    printf("\n");
                    perror("connect");
                    exit(1);
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
