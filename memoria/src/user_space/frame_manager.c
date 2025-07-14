#include "frame_manager.h"

_Atomic uint32_t frames_free_count = 0;
uint32_t frames_total = 0;
t_bitarray *frames_bitmap = NULL;
pthread_mutex_t frames_mutex;

int frame_get_free_count() {
    return frames_free_count;
}

void frame_allocation_init()
{
    frames_total = memoria_config.TAM_MEMORIA / memoria_config.TAM_PAGINA;
    frames_free_count = frames_total;

    size_t bitmap_size = frames_total / 8;
    if (frames_total % 8 != 0)
    {
        bitmap_size += 1;
    }

    char *bitmap_data = calloc(bitmap_size, 1); // Initialize all bits to 0 (free)
    frames_bitmap = bitarray_create_with_mode(bitmap_data, bitmap_size, LSB_FIRST);
    
    // Initialize mutex for bitmap access
    pthread_mutex_init(&frames_mutex, NULL);
}

void release_frames(t_list *frame_list)
{
    pthread_mutex_lock(&frames_mutex);
    
    for (int i = 0; i < list_size(frame_list); i++)
    {
        uint32_t *frame_num = list_get(frame_list, i);
        bitarray_clean_bit(frames_bitmap, *frame_num);
        free(frame_num);
    }

    frames_free_count += list_size(frame_list);
    
    pthread_mutex_unlock(&frames_mutex);

    list_destroy(frame_list);
}

t_list* allocate_frames(uint32_t pages_needed) {
    pthread_mutex_lock(&frames_mutex);
    
    if (pages_needed > frames_free_count) {
        pthread_mutex_unlock(&frames_mutex);
        return NULL;
    }

    t_list* allocated_frames = list_create();
    uint32_t frames_allocated = 0;

    for (uint32_t frame_index = 0; frame_index < frames_total && frames_allocated < pages_needed; frame_index++) {
        if (!bitarray_test_bit(frames_bitmap, frame_index)) {
            bitarray_set_bit(frames_bitmap, frame_index);
            
            uint32_t* frame_num = malloc(sizeof(uint32_t));
            *frame_num = frame_index;
            list_add(allocated_frames, frame_num);

            frames_allocated++;
        }
    }

    frames_free_count -= frames_allocated;
    
    pthread_mutex_unlock(&frames_mutex);
    
    return allocated_frames;
}