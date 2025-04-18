#include "server.h"

int create_server(char* port) {

	log_info(get_logger(), "Creating server...");

	int socket_server;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int getaddrinfoErr = getaddrinfo(NULL, port, &hints, &servinfo);
	if (getaddrinfoErr != 0)
	{
		log_error(get_logger(), "getaddrinfo failed");
		return -1;
	}

	socket_server = socket(servinfo->ai_family,
                        servinfo->ai_socktype,
                        servinfo->ai_protocol);

	if(socket_server == -1) {
		log_error(get_logger(), "Could not create fd for socket");
		freeaddrinfo(servinfo);
		return -1;
	}

	int errSockOpt = setsockopt(socket_server, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));
	if(errSockOpt == -1) {
		log_error(get_logger(), "Could not associate multiple sockets to one port");
		close(socket_server);
		freeaddrinfo(servinfo);
		return -1;
	}

	int bindErr = bind(socket_server, servinfo->ai_addr, servinfo->ai_addrlen);
	if(bindErr == -1) {
		log_error(get_logger(), "Could not associate a socket to port");
		close(socket_server);
		freeaddrinfo(servinfo);
		return -1;
	}

	int listenErr = listen(socket_server, SOMAXCONN);
	if(listenErr == -1) {
		log_error(get_logger(), "Could not listen on port %s", port);
		close(socket_server);
		freeaddrinfo(servinfo);
		return -1;
	}

	freeaddrinfo(servinfo);

	return socket_server;
}

int accept_connection(int socket_server)
{
	log_info(get_logger(), "Awaiting for new clients...");

	int client_socket = accept(socket_server, NULL, NULL);
	if (client_socket == -1)
	{
		log_error(get_logger(), "accept failed");
	}

	log_info(get_logger(), "Client connected.");
	
	return client_socket;
}