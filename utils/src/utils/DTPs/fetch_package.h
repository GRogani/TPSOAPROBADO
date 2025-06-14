#ifndef DTP_FETCH_INSTRUCITON_H
#define DTP_FETCH_INSTRUCITON_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"

typedef struct fetch_package_data {
    uint32_t pid;
    uint32_t pc;
} fetch_package_data;

t_package* create_fetch_package(uint32_t pid, uint32_t pc);

void send_fetch_package(int socket, uint32_t pid, uint32_t pc);

fetch_package_data read_fetch_package(t_package* package);


#endif