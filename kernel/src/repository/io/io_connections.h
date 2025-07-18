#ifndef KERNEL_IO_CONNECTIONS_REPOSITORY_H
#define KERNEL_IO_CONNECTIONS_REPOSITORY_H

#include "collections/collections.h"
#include "commons/string.h"
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct t_io_connection
{
  char *device_name;
  int current_process_executing;
} t_io_connection;

// Structure to return connection status information
typedef struct
{
  int socket_id;               // Socket ID of the connection, -1 if not found
  bool is_busy;                // true if connection is busy (current_process_executing != -1) -> podria pasar que este status esté desactualizado, que hacemos en ese caso?
  bool found;                  // true if connection was found
  t_io_connection *connection; // Pointer to the connection found, NULL if not found
} t_io_connection_status;

void initialize_repository_io_connections();
void destroy_repository_io_connections();

void lock_io_connections();
void unlock_io_connections();

// find
void *find_io_connection_by_socket(int);
t_list *find_io_connection_by_device_name(char *);
void *find_free_connection_from_device_name(char *);
t_io_connection_status get_io_connection_status_by_device_name(char *);

// create
void create_io_connection(int, char *);

// update
void update_io_connection_current_processing(int, int);

// delete
void connection_destroyer(void *);
void delete_io_connection(int);

#endif