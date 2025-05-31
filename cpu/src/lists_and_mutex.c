#include  "lists_and_mutex.h"

sem_t cpu_mutex;
sem_t interrupt_list_mutex;
sem_t interrupt;
static t_list* interrupt_list;

void init_list_and_mutex()
{
    sem_init(&interrupt_list_mutex, 0, 1);
    sem_init(&cpu_mutex, 0, 1);
    sem_init(&interrupt, 0, 0);
    interrupt_list = list_create();
}

void lock_interrupt_list()
{
    LOG_DEBUG("Locking interrupt list");
    sem_wait(&interrupt_list_mutex);
}
void unlock_interrupt_list()
{
    LOG_DEBUG("Unlocking interrupt list");
    sem_post(&interrupt_list_mutex);
}

void add_interrupt(t_package* interrupt)
{
    list_add(interrupt_list, (void*)interrupt);
}

t_package* get_last_interrupt(int i)
{
    t_package* interrupt = NULL;

    if (list_size(interrupt_list) > 0) {
        interrupt = (t_package*)list_remove(interrupt_list, i - 1 );
    }

    return interrupt;
}

int interrupt_count()
{
    int count = list_size(interrupt_list);

    return count;
}

void wait_interrupt() 
{
    sem_wait(&interrupt);
}

void signal_interrupt() 
{
    sem_post(&interrupt);
}

void lock_cpu_mutex() 
{
    LOG_DEBUG("Locking CPU mutex");
    sem_wait(&cpu_mutex);
}

void unlock_cpu_mutex() 
{
    LOG_DEBUG("Unlocking CPU mutex");
    sem_post(&cpu_mutex);
}

void destroy_list_and_mutex() 
{
    sem_destroy(&interrupt_list_mutex);
    sem_destroy(&cpu_mutex);
    sem_destroy(&interrupt);
    list_destroy_and_destroy_elements(interrupt_list, (void*)package_destroy);
}