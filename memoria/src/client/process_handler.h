#ifndef PROCESS_HANDLER_H
#define PROCESS_HANDLER_H

#include "../utils.h"
#include "kernel_space/process_manager.h"
#include "user_space/frame_manager.h"

// Virtual to physical address translation
uint32_t translate_address(uint32_t pid, uint32_t virtual_address);

// User memory read/write operations
bool read_user_memory(uint32_t pid, uint32_t virtual_address, void* buffer, size_t size);
bool write_user_memory(uint32_t pid, uint32_t virtual_address, const void* data, size_t size);

// Server Request Handler functions
void init_process_request_handler(int socket, t_package* package);
void get_instruction_request_handler(int socket, t_package* package);
void delete_process_request_handler(int socket, t_package *package);
void get_free_space_request_handler(int socket);

#endif 