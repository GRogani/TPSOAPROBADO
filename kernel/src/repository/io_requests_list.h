#ifndef KERNEL_IO_REQUESTS_LIST_REPOSITORY_H
#define KERNEL_IO_REQUESTS_LIST_REPOSITORY_H

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <lists/lists.h>

bool initialize_repository_io_requests_list();
bool destroy_repository_io_requests_list();

void lock_io_requests_list();

/**
 * @brief devuelve el primer elemento de la lista, que esté asignado al socket provisto. si no hay elementos asignados, entonces devuelve NULL.
*/
void* find_first_element_for_socket(t_list*, int);

/**
 * @brief devuelve el primer elemento de la lista, que NO esté asignado.
*/
void* find_first_element_without_assign(t_list*);

void create_io_request_element(t_list*, int, int);

/**
 * @brief asigna la conexion del socket provisto, a la proxima request que está pendiente a ejecutarse (processing: false). Si no hay ningun proceso a asignar, retornará NULL.
*/
void* assign_connection_to_request(void*, int);

void unlock_io_requests_list();

#endif