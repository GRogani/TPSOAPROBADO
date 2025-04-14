#ifndef UTILS_SOCKET_SERVER
#define UTILS_SOCKET_SERVER

#include "../logger/logger.h"
#include<stdlib.h>
#include<stdio.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>

extern t_log* logger;

/**
* @brief Creates a server and listen at the given port
* @param port port to listen by the server
* @return connection connection of the socket server
*/
int create_server(char* port);

/**
* @brief Waits for a new client to be connected
* @param socket_server connection of the socket server
* @return connection connection of the client
*/
int accept_connection(int socket_server);

#endif