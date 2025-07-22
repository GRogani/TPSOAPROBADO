#ifndef SWAP_SPACE_SWAP_OUT_H
#define SWAP_SPACE_SWAP_OUT_H

#include <stdint.h>
#include "../kernel_space/process_manager.h"
#include "swap_manager.h"
#include "../user_space/user_space_memory.h"
#include "../user_space/frame_manager.h"
#include "../semaphores.h"

/**
 * @brief Suspend a process - move its pages from user space to swap space.
 * @param pid Process ID of the process to suspend.
 * @return 0 on success, -1 on failure.
 */
bool swap_out_process(int32_t pid);

#endif
