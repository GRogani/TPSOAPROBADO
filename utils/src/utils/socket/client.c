#include "client.h"

int create_connection(char* port, char* ip) {
    struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, port, &hints, &server_info);

	int socket_client = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	int connectErr = connect(socket_client, server_info->ai_addr, server_info->ai_addrlen);
	if(connectErr == -1) {
		log_error(logger, "Could not create connection to server");
		return -1;
	}

	freeaddrinfo(server_info);

	return socket_client;
}