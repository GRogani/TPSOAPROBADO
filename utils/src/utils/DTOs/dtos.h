#ifndef UTILS_DTOS_H
#define UTILS_DTOS_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"



char* read_handshake(t_package* package);
t_package* create_handshake(char* yourName);
int send_handshake(int socket, char* yourName);

#endif