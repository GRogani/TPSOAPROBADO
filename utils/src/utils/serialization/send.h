#ifndef SEND_MESSAGE_H
#define SEND_MESSAGE_H

#include "package.h"
#include <sys/socket.h>
#include <netdb.h>
#include <commons/collections/list.h>

int sendMessageFromStruct(int socket_connection, OPCODE opcode, void* generic_struct, int structFieldsCount);
OPCODE receiveMessageFromStruct(int socket_connection, void* generic_struct, int structFieldsCount);

#endif 