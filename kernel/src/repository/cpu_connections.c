#include "cpu_connections.h"

static sem_t sem_cpu_connections;

bool initialize_repository_cpu_connections()
{
  if (sem_init(&sem_cpu_connections, 0, 1) != 0)
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