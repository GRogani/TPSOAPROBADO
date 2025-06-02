#include "tlb&cache.h"

uint32_t buscar_en_TLB(uint32_t num_page){
    uint32_t num_frame = 0;
    extern t_dictionary *TLB;
    char* key;
    asprintf(&key, "%d", num_page);

    if (dictionary_has_key(TLB, key)){
        num_frame = atoi(dictionary_get(TLB, key));
        free(key);
        LOG_DEBUG("TLB hit: Page %d found in frame %d", num_page, num_frame);
    }
    else {num_frame = NULL;}

    return num_frame;

}