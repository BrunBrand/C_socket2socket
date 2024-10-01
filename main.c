/*
 *  socket2socket main program
 *
 *  How it works:
 *      The program aims to create a server on the local machine that forwards
 *      incoming messages to another server. Formally this can be used to establish
 *      connections between a machine that has no contact with the Internet, but connects
 *      to a proxy server that is connected to the Internet.
 *
 *  How to use:
 *      Give the address and port of the server to forward messages to, and
 *      the port to which the local server will listen for messages to be forwarded
 *
 *
 *  TODO: reformat and organize code
 *
 *  TODO: Create GUI application with GTK
 */

#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include "socket2socket_lib/socket2socket_lib.h"


int main(const int argc, char **argv) {


#ifdef _WIN32
    WSADATA d;
    if(WSAStartup(MAKEWORD(2,2), &d)){
        Error("Failed to initialize");
    }
#endif

    printf("\n");

    if (argc < 4 || argc > 5) {
        printf("Usage: socket2socket remote_host remote_port local_port\n\n");
        exit(1);
    }

    printf("Initialised.\n");

    const char *remote_host = argv[1];
    const char *remote_port = argv[2];
    const char *local_port = argv[3];

    run_server(remote_host, remote_port, local_port);

#ifdef _WIN32
    WSACleanup();
#endif

    exit(0);
}
