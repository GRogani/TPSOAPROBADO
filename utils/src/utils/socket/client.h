#ifndef UTILS_SOCKET_CLIENT
#define UTILS_SOCKET_CLIENT

#include "../logger/logger.h"
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>

extern t_log* logger;

/**
* @brief Creates a client and connects to a socket server
* @param port port to connect
* @param ip ip to connect
* @return connection connection of the socket server
*/
int create_connection(char* port, char* ip);

#endif