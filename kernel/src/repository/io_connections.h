#ifndef KERNEL_IO_CONNECTIONS_REPOSITORY_H
#define KERNEL_IO_CONNECTIONS_REPOSITORY_H

#include "collections/collections.h"
#include "commons/string.h"
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

void initialize_repository_io_connections();
void destroy_repository_io_connections();

void lock_io_connections();
void unlock_io_connections();

// find
void *find_io_connection_by_socket(int);
t_list *find_io_connection_by_device_name(char *);
void *find_free_connection_from_device_name(char *);

// create
void create_io_connection(int, char *);

// update
void update_io_connection_current_processing(int, int);

// delete
void connection_destroyer(void *);
void delete_io_connection(int);

#endif