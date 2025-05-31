#include "io_requests_queue.h"

void initialize_repository_io_requests_queue(sem_t *sem_io_requests_queue)
{
    if (sem_init(sem_io_requests_queue, 0, 1) != 0)
    {
        LOG_ERROR("sem_init for IO_REQUESTS_QUEUE failed");
        exit(EXIT_FAILURE);
    }
}

void destroy_repository_io_requests_queue(sem_t *sem_io_requests_queue)
{
    sem_destroy(sem_io_requests_queue);
}

void lock_io_requests_queue(sem_t *sem_io_requests_queue)
{
    sem_wait(sem_io_requests_queue);
}

void create_io_request_element(t_queue *queue, int process_id, int sleep_time)
{
    t_io_request *io_request = safe_malloc(sizeof(t_io_request));
    io_request->pid = process_id;
    io_request->sleep_time = sleep_time;

    queue_push(queue, io_request);
}

void *get_next_request_in_queue(t_queue *queue)
{
    int size = queue_size(queue);
    if (size == 0)
    {
        return NULL;
    }

    return queue_peek(queue);
}

void *pop_next_request_in_queue(t_queue *queue)
{
    int size = queue_size(queue);
    if (size == 0)
    {
        return NULL;
    }

    return queue_pop(queue);
}

void unlock_io_requests_queue(sem_t *sem_io_requests_queue)
{
    sem_post(sem_io_requests_queue);
}