#ifndef SWAP_SPACE_SWAP_IN_H
#define SWAP_SPACE_SWAP_IN_H

#include <stdint.h>
#include "../kernel_space/process_manager.h"

/**
 * @brief Resume a process - move its pages from swap space back to user space.
 * @param pid Process ID of the process to resume.
 * @return 0 on success, -1 on failure.
 */
int swap_in_process(uint32_t pid);

#endif
