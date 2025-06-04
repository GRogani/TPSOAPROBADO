#ifndef UTILS_CPU_DISPATCH_H
#define UTILS_CPU_DISPATCH_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"

typedef struct dispatch_package_data {
    uint32_t pid;
    uint32_t pc;
} dispatch_package_data;

/**
 * @brief 
 */
dispatch_package_data read_dispatch_package(t_package* package);

/**
 * @brief 
 */
t_package* create_dispatch_package(uint32_t pid, uint32_t pc);


/**
 * @brief 
 */
int send_dispatch_package(int socket, uint32_t pid, uint32_t pc);


#endif
