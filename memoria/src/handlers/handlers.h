#ifndef HANDLERS_H
#define HANDLERS_H

#include <pthread.h>
#include "../utils.h"

/// @brief Crea el hilo principal del servidor que escucha conexiones entrantes.
/// @param listener_thread Puntero al identificador del hilo creado.
/// @return 0 si se creó correctamente, o el código de error de pthread_create.
int create_server_thread(pthread_t* listener_thread);

/// @brief Hilo principal que escucha conexiones en el puerto configurado.
/// @details Por cada nueva conexión, crea un hilo para manejar al cliente.
/// @param arg No utilizado.
/// @return Siempre NULL.
void* client_listener(void* arg);

/// @brief Hilo que maneja la conexión de un cliente (Kernel o CPU).
/// @details Determina el tipo de cliente por el header recibido y llama al handler adecuado.
/// @param client_fd_ptr Puntero a file descriptor del cliente (reservado dinámicamente).
/// @return Siempre NULL.
void* client_handler(void* client_fd_ptr);


#endif