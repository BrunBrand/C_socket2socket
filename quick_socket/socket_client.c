//
// Created by brunob on 9/26/24.
//

#include "socket_client.h"

SOCKET setup_tcp_client_socket(const char *host, const char *port) {

	struct addrinfo hints={0};

	hints.ai_family 	= AF_UNSPEC;
	hints.ai_socktype 	= SOCK_STREAM;
	hints.ai_protocol 	= IPPROTO_TCP;

	struct addrinfo *servAddr;

	int value = 0;
	if ((value = getaddrinfo(host, port, &hints, &servAddr)) !=0){
		exit_with_user_msg("getaddrinfo() failed", gai_strerror(value));
	}

	SOCKET socket_client = INVALID_SOCKET;

	for(const struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next){

		if ((socket_client = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) == INVALID_SOCKET)
			continue;

		if(connect(socket_client, addr->ai_addr, addr->ai_addrlen) == 0) break;

		perror("connect");
		CLOSESOCKET(socket_client);
		socket_client = INVALID_SOCKET;
	}

	freeaddrinfo(servAddr);
	return socket_client;
}

void open_send_close(const char *host, const char *port, const char *message, int len){
	const SOCKET socket_client = setup_tcp_client_socket(host, port);

   	if(send(socket_client, message, len, 0) <= 0) exit_with_sys_msg("send() failed");

   	CLOSESOCKET(socket_client);
}