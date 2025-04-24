#ifndef UTILS_DTOS_H
#define UTILS_DTOS_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"


t_package* create_handshake(char* yourName);

char* read_handshake(t_package* package);

/**
 * @brief Manda un mensaje de handshake
 * @note srteam: `[HANDSHAKE]-[size of stream]-[strlen+1 + "string"]`
*/
int send_handshake(int socket, char* yourName);

#endif