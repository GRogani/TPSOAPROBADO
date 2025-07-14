#ifndef CLIENT_UNSUSPEND_PROCESS_H
#define CLIENT_UNSUSPEND_PROCESS_H

#include <stdint.h>
#include "../utils.h"
#include "../../swap_space/swap_in.h"

/**
 * @brief Handle an unsuspend process request from the kernel.
 * 
 * @param client_fd The client file descriptor.
 * @param package The received package.
 */
void unsuspend_process_request_handler(int client_fd, t_package *package);

#endif
