//
// Created by brunob on 9/26/24.
//

#include "socket_util.h"


void print_socket_address(const struct sockaddr *addr, FILE *stream) {
    if(addr == NULL || stream == NULL) return;

    void *address_numeric;

    char address_buff[INET6_ADDRSTRLEN];

    in_port_t port;

    switch(addr->sa_family){
        case AF_INET:
            address_numeric = &((struct sockaddr_in *) addr)->sin_addr;
            port = ntohs(((struct sockaddr_in *) addr)->sin_port);
        break;
        case AF_INET6:
            address_numeric = &((struct sockaddr_in6 *) addr)->sin6_addr;
            port = ntohs(((struct sockaddr_in6 *) addr)->sin6_port);
        break;
        default:
            fputs("[unknown type]", stream);
        return;
    }

    if(inet_ntop(addr->sa_family, address_numeric, address_buff, sizeof(address_buff)) == NULL){
        fputs("[invalid address]", stream);
    } else{
        fprintf(stream, "%s", address_buff);
        if(port != 0){
            fprintf(stream, "-%u", port);
        }
    }
}

void exit_with_sys_msg(const char* msg) {
    perror(msg);
    exit(1);
}


void exit_with_user_msg(const char *msg, const char *detail){
    fputs(msg, stderr);
    fputs(": ", stderr);
    fputs(detail, stderr);
    fputc('\n', stderr);
    exit(1);
}
