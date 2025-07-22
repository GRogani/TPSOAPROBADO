#ifndef GET_PAGE_ENTRY_H
#define GET_PAGE_ENTRY_H

#include "../utils.h"
#include "../../kernel_space/process_manager.h"
#include "../../kernel_space/page_table.h"

void handle_page_walk_request(int client_socket, t_package *package);

t_page_table *find_table_by_id(t_page_table *root_table, int32_t target_id);

#endif