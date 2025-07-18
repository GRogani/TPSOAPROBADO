#include "delay_utils.h"

extern CacheConfig *g_cache_config;

void delay_cache_access(void)
{
  LOG_DEBUG("Applying cache delay: %d ms", g_cache_config->retardo_cache);
  usleep(g_cache_config->retardo_cache * 1000); // Convert ms to us
}
