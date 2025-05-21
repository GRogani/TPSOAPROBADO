#include "client.h"

int create_connection(char* port, char* ip) {

    struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;   		//IPV4
	hints.ai_socktype = SOCK_STREAM;	//TCP
	hints.ai_flags = AI_PASSIVE;

	int getaddrinfoErr = getaddrinfo(ip, port, &hints, &server_info);
	if (getaddrinfoErr != 0)
	{
		log_error(get_logger(), "geraddrinfo(...) failed");
		freeaddrinfo(server_info);
		return -1;
	}

	int socket_client = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
	if (socket_client == -1)
	{
		log_error(get_logger(), "Socket creation failed");
		freeaddrinfo(server_info);
		return -1;
	}

	int connectErr = connect(socket_client, server_info->ai_addr, server_info->ai_addrlen);
	if (connectErr == -1) {

		log_error(get_logger(), "Could not create connection to server.\tIP:%s PORT:%s", ip, port);

		return -1;
	}

	freeaddrinfo(server_info);

	return socket_client;
}