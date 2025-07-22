#ifndef DTP_PAGE_ENTRY_H
#define DTP_PAGE_ENTRY_H

#include "utils/logger/logger.h"
#include "utils/serialization/package.h"
#include "utils/serialization/buffer.h"
#include "enums/Eopcodes.h"

typedef struct page_entry_request_data {
    int32_t pid;
    int32_t table_id;
    int32_t entry_index;
} page_entry_request_data;

typedef struct page_entry_response_data {
    int32_t value; // table_ptr or frame_number
    bool is_last_level;
} page_entry_response_data;

t_package* create_page_entry_request_package(int32_t pid, int32_t table_level, int32_t entry_index);

void send_page_entry_request_package(int socket, int32_t pid, int32_t table_level, int32_t entry_index);

page_entry_request_data read_page_entry_request_package(t_package* package);

t_package *create_page_entry_response_package(int32_t value, int32_t is_last_level);

void send_page_entry_response_package(int socket, int32_t value, int32_t is_last_level);

page_entry_response_data read_page_entry_response_package(t_package* package);

#endif
