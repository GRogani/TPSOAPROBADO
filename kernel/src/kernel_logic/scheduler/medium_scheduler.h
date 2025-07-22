#ifndef KERNEL_MEDIUM_SCHEDULER_H
#define KERNEL_MEDIUM_SCHEDULER_H

#include <stdint.h>
#include "../utils.h"
#include "../../collections/collections.h"
#include "long_scheduler.h"
#include "short_scheduler.h"
#include "../../repository/pcb/pcb.h"

extern t_kernel_config kernel_config;

void run_medium_scheduler(int32_t  pid);

#endif