#include "semaphore.h"

static sem_t sem_cpu_connected;
static sem_t sem_can_start;

void initialize_cpu_connected_sem()
{
  if (sem_init(&sem_cpu_connected, 0, 1) != 0)
  {
    LOG_ERROR("sem_init for CPU_CONNECTED failed");
    exit(EXIT_FAILURE);
  }
}

void wait_cpu_connected()
{
  sem_wait(&sem_cpu_connected);
}

void signal_cpu_connected()
{
  sem_post(&sem_cpu_connected);
}

void destroy_cpu_connected_sem()
{
  sem_destroy(&sem_cpu_connected);
}

void initialize_can_start_sem()
{
  if (sem_init(&sem_can_start, 0, 0) != 0)
  {
    LOG_ERROR("sem_init for CAN_START failed");
    exit(EXIT_FAILURE);
  }
}

void wait_can_start()
{
  sem_wait(&sem_can_start);
}

void signal_can_start()
{
  sem_post(&sem_can_start);
}

void destroy_can_start_sem()
{
  sem_destroy(&sem_can_start);
}

void initialize_global_semaphores()
{
  initialize_cpu_connected_sem();
  initialize_can_start_sem();
}