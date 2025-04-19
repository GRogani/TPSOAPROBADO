#include "send.h"

/*
* Struct must be of char* fields only!!
* THIS FUNCTION NOT LIBERATES MEMORY OF STRUCT!! 
*/
int sendMessageFromStruct(int socket_connection, OPCODE opcode, void* generic_struct, int structFieldsCount)
{
    t_buffer* buffer = buffer_create(0); // always reallocate memory
    char** ptr = (char**) generic_struct;

    for (int i = 0; i < structFieldsCount; i++) {
        char* message = ptr[i];
        buffer_add_string(buffer, strlen(message) + 1, message);
    }

    t_package* package = package_create(opcode, buffer);
    int result = send_package(socket_connection, package);
    if(result == -1) {
        return result;
    }

    package_destroy(package);
}

OPCODE receiveMessageFromStruct(int socket_connection, void* generic_struct, int structFieldsCount){
    t_package* package = recv_package(socket_connection);
    
    OPCODE opcode = package->opcode;
    char** ptr = (char**) generic_struct;

    for (int i = 0; i < structFieldsCount; i++) {
        uint32_t* message_length = malloc(sizeof(uint32_t));
        char* message = buffer_read_string(package->buffer, message_length);
        ptr[i] = message;
        free(message_length);
    }

    package_destroy(package);
    return opcode;
}