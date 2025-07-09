#ifndef UTILS_SOCKET_SERVER_H
#define UTILS_SOCKET_SERVER_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include "../../macros/logger_macro.h"


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
int accept_connection(char* prefix, int socket_server);

#endif