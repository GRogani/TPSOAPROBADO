#ifndef SWAP_SPACE_SWAP_OUT_H
#define SWAP_SPACE_SWAP_OUT_H

#include <stdint.h>
#include "../kernel_space/process_manager.h"

/**
 * @brief Suspend a process - move its pages from user space to swap space.
 * @param pid Process ID of the process to suspend.
 * @return 0 on success, -1 on failure.
 */
int swap_out_process(uint32_t pid);

#endif
