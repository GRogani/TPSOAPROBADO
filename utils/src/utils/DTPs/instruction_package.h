#ifndef DTP_INSTRUCTION_H
#define DTP_INSTRUCTION_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"

t_package *create_instruction_package(char *instruction);

int send_instruction_package(int socket, char* instruction);

char *read_instruction_package(t_package *package);

#endif
