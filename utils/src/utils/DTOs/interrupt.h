#ifndef DTOS_INTERRUPT_H
#define DTOS_INTERRUPT_H


#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"

void send_interrupt(int socket_interrupt, uint32_t PID);


#endif