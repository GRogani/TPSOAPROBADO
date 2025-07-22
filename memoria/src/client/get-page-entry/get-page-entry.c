#include "get-page-entry.h"

void handle_page_walk_request(int client_socket, t_package* package) {
  page_entry_request_data payload = read_page_entry_request_package(package);

  process_info* process = process_manager_find_process(payload.pid);
  
  if (process == NULL) {
    LOG_ERROR("Process with PID %d not found", payload.pid);
    send_page_entry_response_package(client_socket, INVALID_FRAME_NUMBER, 1);
    return;
  }

  process->metrics->page_table_access_count++;

  t_page_table* current_table = process->page_table;
  t_page_table* target_table = NULL;

  if (current_table->table_id == payload.table_id) {
    target_table = current_table;
  } else {
    target_table = find_table_by_id(process->page_table, payload.table_id);
  }

  if (target_table == NULL) {
    LOG_ERROR("Table with ID %d not found for process %d", payload.table_id, payload.pid);
    send_page_entry_response_package(client_socket, INVALID_FRAME_NUMBER, 1);
    return;
  }

  if (payload.entry_index >= target_table->num_entries) {
    LOG_ERROR("Entry index %d out of bounds for table %d (max: %d)", 
              payload.entry_index, payload.table_id, target_table->num_entries - 1);
    send_page_entry_response_package(client_socket, INVALID_FRAME_NUMBER, 1);
    return;
  }

  t_page_table_entry* entry = list_get(target_table->entries, payload.entry_index);
  
  int32_t response_value;
  if (entry->is_last_level) {
    response_value = entry->frame_number;
    LOG_DEBUG("Returning frame number %d for PID %d, table %d, entry %d", 
             response_value, payload.pid, payload.table_id, payload.entry_index);
  } else {
    response_value = entry->next_table_id;
    LOG_DEBUG("Returning next table ID %d for PID %d, table %d, entry %d", 
             response_value, payload.pid, payload.table_id, payload.entry_index);
  }

  send_page_entry_response_package(client_socket, response_value, entry->is_last_level ? 0 : 1);
}

t_page_table* find_table_by_id(t_page_table* root_table, int32_t target_id) {
  if (root_table == NULL) {
    return NULL;
  }

  if (root_table->table_id == target_id) {
    return root_table;
  }

  if (list_size(root_table->entries) > 0) {
    t_page_table_entry* first_entry = list_get(root_table->entries, 0);
    if (first_entry->is_last_level) {
      return NULL;
    }
  }

  for (int i = 0; i < root_table->num_entries; i++) {
    t_page_table_entry* entry = list_get(root_table->entries, i);
    if (!entry->is_last_level && entry->next_table != NULL) {
      t_page_table* found = find_table_by_id(entry->next_table, target_id);
      if (found != NULL) {
        return found;
      }
    }
  }

  return NULL;
}