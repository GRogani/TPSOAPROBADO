#ifndef READ_MEMORY_H
#define READ_MEMORY_H

#include "../utils.h"

/**
 * @brief Handles read memory requests from clients.
 * 
 * @param client_socket The socket connected to the client.
 * @param package The received package containing the read memory request.
 */
void read_memory_request_handler(int client_socket, t_package* package);

#endif
