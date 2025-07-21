#ifndef ASSIGN_FRAMES_TO_PROCESS_H
#define ASSIGN_FRAMES_TO_PROCESS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include "page_table.h"

typedef struct {
    t_page_table *table;
    int level;
} queue_item_t;

typedef struct
{
  t_page_table *table;
  int level;
  int entry_index;
} stack_item_t;

static void collect_last_level_tables(t_page_table *root_table, stack_item_t *result, int *result_size, int max_result_size);

bool assign_frames(t_page_table *root_table, t_list *frames_for_process, int pages_needed);

#endif