#include "cpu.h"

void initialize_repository_cpu(sem_t *sem_cpu)
{
  if (sem_init(sem_cpu, 0, 1) != 0)
  {
    LOG_ERROR("sem_init for CPU failed");
    exit(EXIT_FAILURE);
  }
}

void destroy_repository_cpu(sem_t *sem_cpu)
{
  sem_destroy(sem_cpu);
}

void lock_cpu(sem_t *sem_cpu)
{
  sem_wait(sem_cpu);
}

void unlock_cpu(sem_t *sem_cpu)
{
  sem_post(sem_cpu);
}