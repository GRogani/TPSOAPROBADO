#ifndef DTP_CONFIRMATION_PACKAGE_H
#define DTP_CONFIRMATION_PACKAGE_H

#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"


int read_confirmation_package(t_package *package);

t_package *create_confirmation_package(int success);

int send_confirmation_package(int socket, uint32_t success);


#endif