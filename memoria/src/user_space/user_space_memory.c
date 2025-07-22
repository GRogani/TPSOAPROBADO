#include "user_space_memory.h"

static char *user_space = NULL;
static size_t user_space_size = 0;
extern t_memoria_config memoria_config;

pthread_mutex_t user_space_mutex;

void init_user_space(size_t size)
{
    pthread_mutex_init(&user_space_mutex, NULL);

    user_space = safe_calloc(1, size);
    user_space_size = size;

    LOG_INFO("Memoria de usuario inicializada con: %zu bytes", size);
}

void destroy_user_space(void)
{
    if (user_space != NULL)
    {
        free(user_space);
        user_space = NULL;
        user_space_size = 0;

        pthread_mutex_destroy(&user_space_mutex);
        LOG_INFO("Memoria de usuario destruida");
    }
}

void write_to_user_space(int32_t physical_address, void *data, int32_t size)
{
    if (size > memoria_config.TAM_PAGINA)
    {
        LOG_ERROR("Size de escritura %u excede size de frame %d", size, memoria_config.TAM_PAGINA);
        return;
    }

    if (physical_address + size > user_space_size)
    {
        LOG_ERROR("Escritura en direccion Fisica %u de %uB excede el size de espacio de usuario %zu",
                  physical_address, size, user_space_size);
        return;
    }
    
    pthread_mutex_lock(&user_space_mutex);

    memcpy(&user_space[physical_address], data, size);

    pthread_mutex_unlock(&user_space_mutex);

    LOG_DEBUG("Se escribieron %uB en la direccion fisica %u", size, physical_address);
}

void read_from_user_space(int32_t physical_address, void *buffer, int32_t size)
{
    if (physical_address + size > user_space_size)
    {
        LOG_ERROR("Lectura desde direccion fisica %u de %uB excede el size de espacio de usuario %zu",
                  physical_address, size, user_space_size);
        memset(buffer, 0, size);
        return;
    }

    pthread_mutex_lock(&user_space_mutex);

    memcpy(buffer, &user_space[physical_address], size);

    pthread_mutex_unlock(&user_space_mutex);

    LOG_DEBUG("Leidos %uB desde la direccion fisica %u", size, physical_address);
}

size_t get_user_space_size(void)
{
    return user_space_size;
}
