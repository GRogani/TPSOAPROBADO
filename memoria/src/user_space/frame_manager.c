#include "frame_manager.h"

_Atomic int32_t frames_free_count = 0;
int32_t frames_total = 0;
t_bitarray *frames_bitmap = NULL;
pthread_mutex_t frames_mutex;

int frame_get_free_count() {
    return frames_free_count;
}

void frame_allocation_init(t_memoria_config memoria_config)
{
    frames_total = memoria_config.TAM_MEMORIA / memoria_config.TAM_PAGINA;
    frames_free_count = frames_total;

    size_t bitmap_size = frames_total / 8;
    if (frames_total % 8 != 0)
    {
        bitmap_size += 1;
    }

    char *bitmap_data = safe_calloc(bitmap_size, sizeof(char));
    frames_bitmap = bitarray_create_with_mode(bitmap_data, bitmap_size, LSB_FIRST);
    
    pthread_mutex_init(&frames_mutex, NULL);
}

void release_frames(t_list *frame_list)
{
    pthread_mutex_lock(&frames_mutex);
    
    // Guardamos el tamaño de la lista antes de modificarla
    int frames_released = list_size(frame_list);
    
    for (int i = 0; i < frames_released; i++)
    {
        int32_t *frame_num = list_get(frame_list, i);
        bitarray_clean_bit(frames_bitmap, *frame_num);
    }

    list_destroy_and_destroy_elements(frame_list, free);

    frames_free_count += frames_released;
    
    pthread_mutex_unlock(&frames_mutex);

}

t_list* allocate_frames(int32_t pages_needed) {
    pthread_mutex_lock(&frames_mutex);
    
    if (pages_needed > frame_get_free_count()) {
        LOG_WARNING("No hay suficientes frames para asignar %u paginas. Frames disponibles: %u", pages_needed, frames_free_count);
        pthread_mutex_unlock(&frames_mutex);
        return NULL;
    }

    t_list* allocated_frames = list_create();
    int32_t frames_allocated = 0;

    for (int32_t frame_index = 0; frame_index < frames_total && frames_allocated < pages_needed; frame_index++) {
        if (!bitarray_test_bit(frames_bitmap, frame_index)) {
            bitarray_set_bit(frames_bitmap, frame_index);
            
            int32_t* frame_num = safe_malloc(sizeof(int32_t));
            *frame_num = frame_index;
            list_add(allocated_frames, frame_num);

            frames_allocated++;
        }
    }

    frames_free_count -= frames_allocated;
    
    pthread_mutex_unlock(&frames_mutex);
    
    return allocated_frames;
}