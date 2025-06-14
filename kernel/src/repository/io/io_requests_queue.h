#ifndef KERNEL_IO_REQUESTS_QUEUE_REPOSITORY_H
#define KERNEL_IO_REQUESTS_QUEUE_REPOSITORY_H

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <collections/collections.h>

void initialize_repository_io_requests_queue(sem_t *);
void destroy_repository_io_requests_queue(sem_t *);

void lock_io_requests_queue(sem_t*);

void create_io_request_element(t_queue*, int, int);

// peek the first element of the queue
void *get_next_request_in_queue(t_queue*);

// returns and extract the first element of the queue
void *pop_next_request_in_queue(t_queue *);

void unlock_io_requests_queue(sem_t*);

#endif