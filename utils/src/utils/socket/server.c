#include "server.h"

int create_server(char* port) {
	int socket_server;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int getaddrinfoErr = getaddrinfo(NULL, port, &hints, &servinfo);
	if (getaddrinfoErr != 0)
	{
		LOG_ERROR("getaddrinfo failed");
		return -1;
	}

	socket_server = socket(servinfo->ai_family,
                        servinfo->ai_socktype,
                        servinfo->ai_protocol);

	if(socket_server == -1) {
		LOG_ERROR("Could not create fd for socket");
		freeaddrinfo(servinfo);
		return -1;
	}

	int errSockOpt = setsockopt(socket_server, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));
	if(errSockOpt == -1) {
		LOG_ERROR("Could not associate multiple sockets to one port");
		close(socket_server);
		freeaddrinfo(servinfo);
		return -1;
	}

	int bindErr = bind(socket_server, servinfo->ai_addr, servinfo->ai_addrlen);
	if(bindErr == -1) {
		LOG_ERROR("Could not associate a socket to port");
		close(socket_server);
		freeaddrinfo(servinfo);
		return -1;
	}

	int listenErr = listen(socket_server, SOMAXCONN);
	if(listenErr == -1) {
		LOG_ERROR("Could not listen on port %s", port);
		close(socket_server);
		freeaddrinfo(servinfo);
		return -1;
	}

	freeaddrinfo(servinfo);

	return socket_server;
}

int accept_connection(char* prefix, int socket_server)
{
	LOG_INFO("[%s] Awaiting for new clients...", prefix);

	int client_socket = accept(socket_server, NULL, NULL);
	if (client_socket == -1)
	{
		LOG_ERROR("[%s] Accept failed for server socket: %d", prefix, socket_server);
		return client_socket;
	}

	LOG_INFO("[%s] Client connected. Socket fd: %d", prefix, client_socket);
	
	return client_socket;
}