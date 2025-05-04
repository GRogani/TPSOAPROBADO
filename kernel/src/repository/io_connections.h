#ifndef KERNEL_IO_CONNECTIONS_REPOSITORY_H
#define KERNEL_IO_CONNECTIONS_REPOSITORY_H

#include "lists/lists.h"
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

bool initialize_repository_io_connections();
bool destroy_repository_io_connections();

void lock_io_connection_list();

/*
* @brief finds a connection by provided socket and locks the list. remember to unlock it by yourself
* @returns a socket connection or NULL if no connection found
*/
void* find_and_lock_io_connection_list_by_socket(int);

/*
* @brief should free the returned list by itself after finishing its use.
* @returns list of sockets found for the device_name
*/
t_list* find_and_lock_io_connections_list_by_device_name(char*);

void create_io_connection(int, char*);
void connection_destroyer(void*);
void delete_io_connection(int);
void unlock_io_connections_list();

#endif