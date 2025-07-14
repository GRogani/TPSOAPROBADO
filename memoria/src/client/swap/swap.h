#ifndef CLIENT_SWAP_H
#define CLIENT_SWAP_H

#include <stdint.h>
#include "../utils.h"
#include "../../swap_space/swap_out.h"

/**
 * @brief Handle a swap request from the kernel.
 * 
 * @param client_fd The client file descriptor.
 * @param package The received package.
 */
void swap_request_handler(int client_fd, t_package *package);

#endif
