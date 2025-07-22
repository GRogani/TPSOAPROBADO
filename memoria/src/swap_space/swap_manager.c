#include "swap_manager.h"

#define SWAP_INITIAL_FRAMES 4096

extern t_memoria_config memoria_config;

static FILE *swap_file = NULL;
static t_bitarray *swap_frames_bitmap = NULL;
static size_t swap_frames_total = 0;
static _Atomic int32_t swap_frames_free_count = 0;
static pthread_mutex_t swap_frames_mutex;
static pthread_mutex_t swap_file_mutex;

t_list *swap_allocate_frames(int32_t pages_needed)
{
  pthread_mutex_lock(&swap_frames_mutex);

  if (pages_needed > swap_frames_free_count)
  {
    pthread_mutex_unlock(&swap_frames_mutex);
    return NULL;
  }

  t_list *allocated_frames = list_create();
  int32_t frames_allocated = 0;

  for (int32_t frame_index = 0; frame_index < swap_frames_total && frames_allocated < pages_needed; frame_index++)
  {
    if (!bitarray_test_bit(swap_frames_bitmap, frame_index))
    {
      bitarray_set_bit(swap_frames_bitmap, frame_index);

      int32_t *frame_num = malloc(sizeof(int32_t));
      *frame_num = frame_index;
      list_add(allocated_frames, frame_num);

      frames_allocated++;
    }
  }

  swap_frames_free_count -= frames_allocated;

  pthread_mutex_unlock(&swap_frames_mutex);

  LOG_INFO("Se swapearon %d frames", frames_allocated);
  return allocated_frames;
}

bool swap_write_frame(int32_t frame_num, void *data, int32_t size)
{
  if (swap_file == NULL)
  {
    LOG_ERROR("Swap no inicializado");
    return false;
  }

  pthread_mutex_lock(&swap_file_mutex);

  long position = frame_num * memoria_config.TAM_PAGINA;

  bool result = true;

  if (fseek(swap_file, position, SEEK_SET) != 0)
  {
    LOG_ERROR("Swap: Error seeking to position %ld in swap file", position);
    result = false;
  }
  else
  {
    size_t written = fwrite(data, 1, size, swap_file);
    if (written != size)
    {
      LOG_ERROR("Swap: Error writing to swap file. Expected to write %u bytes, wrote %zu", size, written);
      result = false;
    }
    else
    {
      fflush(swap_file);
      LOG_DEBUG("Swap: Written %u bytes to frame %u at position %ld", size, frame_num, position);
    }
  }

  pthread_mutex_unlock(&swap_file_mutex);

  return result;
}

void swap_manager_init(t_memoria_config memoria_config)
{
  pthread_mutex_init(&swap_frames_mutex, NULL);
  pthread_mutex_init(&swap_file_mutex, NULL);

  swap_file = fopen(memoria_config.PATH_SWAPFILE, "wb+");
  if (swap_file == NULL)
  {
    LOG_ERROR("Swap: Failed to create swap file at %s", memoria_config.PATH_SWAPFILE);
    exit(EXIT_FAILURE);
  }

  swap_frames_total = SWAP_INITIAL_FRAMES;
  swap_frames_free_count = swap_frames_total;

  size_t bitmap_size = swap_frames_total / 8;
  if (swap_frames_total % 8 != 0)
  {
    bitmap_size += 1;
  }

  char *bitmap_data = safe_calloc(bitmap_size, 1);
  swap_frames_bitmap = bitarray_create_with_mode(bitmap_data, bitmap_size, LSB_FIRST);

  LOG_INFO("Swap Manager inicializado. Swap file: %s, espacio inicial: %zu frames", memoria_config.PATH_SWAPFILE, swap_frames_total);
}

void swap_manager_destroy()
{
  if (swap_file != NULL)
  {
    fclose(swap_file);
    swap_file = NULL;
  }

  if (swap_frames_bitmap != NULL)
  {
    free(swap_frames_bitmap->bitarray);
    bitarray_destroy(swap_frames_bitmap);
    swap_frames_bitmap = NULL;
  }

  pthread_mutex_destroy(&swap_frames_mutex);
  pthread_mutex_destroy(&swap_file_mutex);

  LOG_INFO("Swap: Manager destroyed");
}

/**
 * Read data from a specific frame in swap space
 */
bool swap_read_frame(int32_t frame_num, void *buffer, int32_t size)
{
  
  if (swap_file == NULL)
  {
    LOG_ERROR("Swap: Swap file not initialized");
    return false;
  }

  pthread_mutex_lock(&swap_file_mutex);

  // Calculate position (physic dir) in swap file
  long position = frame_num * memoria_config.TAM_PAGINA;

  int result = true;

  if (fseek(swap_file, position, SEEK_SET) != 0)
  {
    LOG_ERROR("Swap: Error seeking to position %ld in swap file", position);
    result = false;
  }
  else
  {
    size_t read_bytes = fread(buffer, 1, size, swap_file);
    if (read_bytes != size)
    {
      LOG_ERROR("Swap: Error reading from swap file. Expected to read %u bytes, read %zu", size, read_bytes);
      result = false;
    }
    else
    {
      LOG_DEBUG("Swap: Read %u bytes from frame %u at position %ld", size, frame_num, position);
    }
  }

  pthread_mutex_unlock(&swap_file_mutex);

  return result;
}

/**
 * Release allocated frames in swap space
 */
void swap_release_frames(t_list *frame_list)
{
  if (frame_list == NULL)
  {
    return;
  }

  pthread_mutex_lock(&swap_frames_mutex);
  
  // Guardamos el tamaño de la lista antes de modificarla
  int frames_released = list_size(frame_list);

  for (int i = 0; i < frames_released; i++)
  {
    int32_t *frame_num = list_get(frame_list, i);
    bitarray_clean_bit(swap_frames_bitmap, *frame_num);
    free(frame_num);
  }

  swap_frames_free_count += frames_released;

  pthread_mutex_unlock(&swap_frames_mutex);

  list_destroy(frame_list);

  LOG_INFO("Swap: Released %d frames in swap space", frames_released);
}