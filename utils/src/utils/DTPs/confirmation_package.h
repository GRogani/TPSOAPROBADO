#ifndef DTP_CONFIRMATION_H
#define DTP_CONFIRMATION_H

#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"

t_package *create_confirmation_package (int32_t success);

int send_confirmation_package (int socket, bool success);

bool read_confirmation_package (t_package *package);


#endif