#ifndef CACHE_H
#define CACHE_H

#include <stdint.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <config_loader.h>


typedef struct {
    bool is_valid;
    uint32_t frame;
    void* content;
    bool use_bit;
    bool modified_bit;
 } CacheEntry;

#endif 