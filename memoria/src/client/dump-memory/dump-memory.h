#ifndef DUMP_MEMORY_H
#define DUMP_MEMORY_H

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../utils.h"
#include "utils/DTPs/dump_memory_package.h"
#include "utils/DTPs/confirmation_package.h"
#include "../../kernel_space/process_manager.h"
#include "../../user_space/frame_manager.h"
#include "../../user_space/user_space_memory.h"

/**
 * @brief Maneja una solicitud de volcado de memoria (dump) para un proceso específico
 */
void dump_memory_request_handler(int client_socket, t_package* package);

FILE *create_dump_file(int32_t pid);

#endif
