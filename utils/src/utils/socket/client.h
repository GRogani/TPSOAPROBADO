#ifndef UTILS_SOCKET_CLIENT_H
#define UTILS_SOCKET_CLIENT_H

#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include "../../macros/logger_macro.h"

/**
* @brief Creates a client and connects to a socket server
* @param port port to connect
* @param ip ip to connect
* @return connection connection of the socket server or -1 if connection fails
*/
int create_connection(char* port, char* ip);

#endif