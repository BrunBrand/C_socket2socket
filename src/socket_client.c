//
// Created by brunob on 9/26/24.
//

#include "socket_client.h"

int setup_tcp_client_socket(const char *host, const char *port) {

	struct addrinfo hints={0};

	hints.ai_family 	= AF_UNSPEC;
	hints.ai_socktype 	= SOCK_STREAM;
	hints.ai_protocol 	= IPPROTO_TCP;

	struct addrinfo *servAddr;

	int value = 0;
	if ((value = getaddrinfo(host, port, &hints, &servAddr)) !=0){
		exit_with_user_msg("getaddrinfo() failed", gai_strerror(value));
	}

	int socket_client = -1;

	for(const struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next){
		socket_client = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if(socket_client < 0) continue;

		if(connect(socket_client, addr->ai_addr, addr->ai_addrlen) == 0) break;

		close(socket_client);
		socket_client = -1;
	}

	freeaddrinfo(servAddr);
	return socket_client;
}