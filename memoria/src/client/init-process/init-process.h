#ifndef INIT_PROCESS_H
#define INIT_PROCESS_H

#include "../utils.h"
#include "utils/DTPs/init_process_package.h"
#include "../../kernel_space/process_manager.h"
#include "../../kernel_space/page_table.h"
#include "../../kernel_space/assign-frames-to-process.h"
#include "../../user_space/frame_manager.h"
#include "../../semaphores.h"

extern t_memoria_config memoria_config;

void init_process_request_handler(int socket, t_package *package);

bool create_process(int32_t pid, int32_t size, char *script_path);

t_list *process_manager_load_script_lines(char *path);

#endif