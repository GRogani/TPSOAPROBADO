#include "server.h"

int create_server(char* port) {
	int socket_server;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, port, &hints, &servinfo);

	socket_server = socket(servinfo->ai_family,
                        servinfo->ai_socktype,
                        servinfo->ai_protocol);

	if(socket_server == -1) {
		log_error(logger, "Could not create fd for socket");
		exit(EXIT_FAILURE);
	}

	int errSOckOpt = setsockopt(socket_server, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));
	if(errSOckOpt == -1) {
		log_error(logger, "Could not associate multiple sockets to one port");
		exit(EXIT_FAILURE);
	}

	int bindErr = bind(socket_server, servinfo->ai_addr, servinfo->ai_addrlen);
	if(bindErr == -1) {
		log_error(logger, "Could not associate a socket to port");
		exit(EXIT_FAILURE);
	}

	int listenErr = listen(socket_server, SOMAXCONN);
	if(listenErr == -1) {
		log_error(logger, "Could not listen on port %s", port);
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(servinfo);

	return socket_server;
}

int accept_connection(int socket_server)
{
	int client_socket = accept(socket_server, NULL, NULL);
	return client_socket;
}