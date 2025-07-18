#include "delay_utils.h"

extern t_memoria_config memoria_config;

void delay_memory_access(void)
{
  LOG_DEBUG("Aplicando retardo de memoria: %d ms", memoria_config.RETARDO_MEMORIA);
  usleep(memoria_config.RETARDO_MEMORIA * 1000);
}

void delay_swap_access(void)
{
  LOG_DEBUG("Aplicando retardo de swap: %d ms", memoria_config.RETARDO_SWAP);
  usleep(memoria_config.RETARDO_SWAP * 1000);
}
