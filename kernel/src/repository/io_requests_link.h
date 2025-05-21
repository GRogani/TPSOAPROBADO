#ifndef KERNEL_IO_REQUESTS_LINK_REPOSITORY_H
#define KERNEL_IO_REQUESTS_LINK_REPOSITORY_H

#include "collections/collections.h"
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

void initialize_repository_io_requests_link();
void destroy_repository_io_requests_link();

void lock_io_requests_link();

/**
 * @brief finds the link between the device_name and the connection. if the link exists, then returns the element. otherwise, returns NULL
*/
void* find_io_request_by_device_name(char*);

/**
 * @brief creates a link between the connection and the requests to that connection. Also creates the requests list for this connection. This list should be free by yourself
*/
void* create_io_request_link(char*);

void unlock_io_requests_link();

#endif