#include "mmu.h"

t_user_memory user_memory;

void init_user_memory(int size)
{
    user_memory.size = (size_t)size;
    user_memory.user_space = safe_calloc(1, user_memory.size);    
}

void destroy_user_memory()
{
    free(user_memory.user_space);
    user_memory.user_space = NULL;
}