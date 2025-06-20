#ifndef KERNEL_MEDIUM_SCHEDULER_H
#define KERNEL_MEDIUM_SCHEDULER_H

#include <stdint.h>
#include "../utils.h"
#include "../../collections/collections.h"

extern t_kernel_config kernel_config;

void run_medium_scheduler(uint32_t  pid);

#endif