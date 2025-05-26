#include "cpu_connections.h"

static sem_t sem_cpu_connections;
static sem_t sem_id_generator;

unsigned int id_generator = 0;

bool initialize_repository_cpu_connections()
{
  if (sem_init(&sem_cpu_connections, 0, 1) != 0)
  {
    LOG_ERROR("sem_init for CPU_CONNECTIONS list failed");
    exit(EXIT_FAILURE);
  }
  if (sem_init(&sem_id_generator, 0, 1) != 0)
  {
    LOG_ERROR("sem_init for CPU_CONNECTIONS list failed");
    exit(EXIT_FAILURE);
  }
}

bool destroy_repository_cpu_connections()
{
  sem_destroy(&sem_cpu_connections);
}

void lock_cpu_connections()
{
  sem_wait(&sem_cpu_connections);
}

void unlock_cpu_connections()
{
  sem_post(&sem_cpu_connections);
}

void create_cpu_connection(int socket_dispatch, int socket_interrupt)
{
  lock_cpu_connections();

  t_cpu_connection* new_connection = malloc(sizeof(t_cpu_connection));

  new_connection->socket_dispatch = socket_dispatch;
  new_connection->socket_interrupt = socket_interrupt;

  sem_wait(&sem_id_generator);
  new_connection->id = id_generator;
  id_generator++;
  sem_post(&sem_id_generator);
  
  new_connection->current_process_executing = -1;
  new_connection->exec_list = list_create();

  list_add(cpu_connections, new_connection);

  unlock_cpu_connections();
}