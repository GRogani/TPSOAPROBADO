#ifndef DTP_INIT_PROCESS_H
#define DTP_INIT_PROCESS_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"

typedef struct init_process_package_data {
    uint32_t pid;
    uint32_t size;
    char* pseudocode_path;
} init_process_package_data;

t_package* create_init_process_package (uint32_t pid, uint32_t size, char* pseudocode_path);

int send_init_process_package (int socket, uint32_t pid, uint32_t size, char* pseudocode_path);

init_process_package_data* read_init_process_package (t_package* package);

void destroy_init_process_package (init_process_package_data* request);


#endif