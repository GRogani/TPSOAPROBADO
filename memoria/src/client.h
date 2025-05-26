#ifndef CLIENT_H
#define CLIENT_H

#include <commons/config.h>
#include <commons/log.h>
#include "memoria.h"


void* handle_client(int skt);
void handle_kernel();
void handle_cpu();


#endif