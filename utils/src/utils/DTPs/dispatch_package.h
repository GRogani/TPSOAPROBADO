#ifndef DTP_DISPATCH_H
#define DTP_DISPATCH_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"

typedef struct dispatch_package_data {
    int32_t pid;
    int32_t pc;
} dispatch_package_data;

/**
 * @brief 
 */
t_package* create_dispatch_package(int32_t pid, int32_t pc);


/**
 * @brief 
 */
int send_dispatch_package(int socket, int32_t pid, int32_t pc);

/**
 * @brief 
 */
dispatch_package_data read_dispatch_package(t_package* package);


#endif
