#ifndef UTILS_MEMORY_GET_INSTRUCTION_H
#define UTILS_MEMORY_GET_INSTRUCTION_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"

char *read_instruction_package(t_package *package);

t_package *create_instruction_package(char *instruction);

int send_instruction_package(int socket, char* instruction);

#endif
