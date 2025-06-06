#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H
#include "mmu.h"
#include <commons/config.h>


MMUConfig load_mmu_config();

TLBConfig load_tlb_config();

CacheConfig load_cache_config();

#endif 