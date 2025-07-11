#include "config_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

MMUConfig load_mmu_config()
{
  t_config *config = init_config("memoria.config");
  MMUConfig mmu_config;
  mmu_config.page_size = config_get_int_value(config, "TAM_PAGINA");
  mmu_config.page_table_levels = config_get_int_value(config, "CANTIDAD_NIVELES");
  mmu_config.entries_per_table = config_get_int_value(config, "ENTRADAS_POR_TABLA");

  config_destroy(config);
  return mmu_config;
}

TLBConfig load_tlb_config()
{
  t_config *config = init_config("cpu.config");
  TLBConfig tlb_config;
  tlb_config.entry_count = config_get_int_value(config, "ENTRADAS_TLB");
  char *algo_str = config_get_string_value(config, "REEMPLAZO_TLB");
  if (strcmp(algo_str, "FIFO") == 0)
  {
    tlb_config.replacement_algo = TLB_ALGO_FIFO;
  }
  else if (strcmp(algo_str, "LRU") == 0)
  {
    tlb_config.replacement_algo = TLB_ALGO_LRU;
  }
  else
  {
    free(algo_str);
    exit(EXIT_FAILURE);
  }

  config_destroy(config);
  free(algo_str);
  return tlb_config;
}

CacheConfig load_cache_config(t_config *config)
{
  CacheConfig cache_config;
  cache_config.entry_count = config_get_int_value(config, "ENTRADAS_CACHE");

  char *algo_str = config_get_string_value(config, "REEMPLAZO_CACHE");
  if (strcmp(algo_str, "CLOCK") == 0)
  {
    cache_config.replacement_algo = CACHE_ALGO_CLOCK;
  }
  else if (strcmp(algo_str, "CLOCK-M") == 0)
  {
    cache_config.replacement_algo = CACHE_ALGO_CLOCK_M;
  }
  else
  {
    free(algo_str);
    exit(EXIT_FAILURE);
  }

  free(algo_str);
  return cache_config;
}