#ifndef DTP_INTERRUPT_H
#define DTP_INTERRUPT_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"

t_package* create_interrupt_package (uint32_t pid);

int send_interrupt_package (int socket, uint32_t pid);

int read_interrupt_package(t_package *package);


#endif
