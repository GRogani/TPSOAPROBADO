#ifndef CLIENT_H
#define CLIENT_H

#include <commons/config.h>
#include <commons/log.h>
#include "memoria.h"

extern t_log* logger;

void* handle_client(int skt);
void handle_kernel();
void handle_cpu();


#endif