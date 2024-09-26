//
// Created by brunob on 9/26/24.
//

#ifndef PORTABLE_SOCKETS_H
    #define PORTABLE_SOCKETS_H
    #if defined(_WIN32)
        #ifndef _WIN32_WINNT
            #define _WIN32_WINNT 0x0600
        #endif
        #include <winsock2.h>
        #include <ws2tcpip.h>
        #pragma comment(lib, "ws2_32.lib")
        #define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
        #define CLOSESOCKET(s) closesocket(s)
        #define GETSOCKETERRNO() (WSAGetLastError())
    #else
        #include <sys/types.h>
        #include <sys/socket.h>
        #include <netinet/in.h>
        #include <arpa/inet.h>
        #include <netdb.h>
        #include <unistd.h>
        #include <errno.h>
        #define ISVALIDSOCKET(s) ((s) >= 0)
        #define CLOSESOCKET(s) close(s)
        #define SOCKET int
        #define GETSOCKETERRNO() (errno)
    #endif
#endif