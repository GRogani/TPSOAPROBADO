#include "interrupt.h"


void send_interrupt(int socket_interrupt, uint32_t PID) 
{
    t_buffer* buffer = buffer_create(sizeof(uint32_t));
    buffer_add_uint32(buffer, PID);
    t_package* package = package_create(CPU_INTERRUPT, buffer);
    send_package(socket_interrupt, package);
    package_destroy(package);
}