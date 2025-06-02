#ifndef TLB_AND_CACHE
#define TLB_AND_CACHE
#define _GNU_SOURCE
#include <commons/collections/dictionary.h>
#include <stdio.h>
#include <stdlib.h>

t_dictionary *TLB = dictionary_create();

uint32_t buscar_en_TLB(uint32_t num_page);

#endif 