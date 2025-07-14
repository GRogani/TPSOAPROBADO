#ifndef GET_PAGE_ENTRY_H
#define GET_PAGE_ENTRY_H

#include "../utils.h"

void handle_page_walk_request(int client_socket, t_package *package);

t_page_table *find_table_by_id(t_page_table *root_table, uint32_t target_id);

#endif