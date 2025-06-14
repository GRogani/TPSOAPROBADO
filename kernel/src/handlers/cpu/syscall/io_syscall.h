#ifndef KERNEL_IO_SYSCALL_H
#define KERNEL_IO_SYSCALL_H

#include "../utils.h"
#include "repository/process/exit_list.h"
#include "kernel_logic/scheduler/long_scheduler.h"
#include "kernel_logic/scheduler/short_scheduler.h"
#include "repository/pcb/pcb.h"
#include "exit_syscall.h"
#include "../../io/process_pending_io.h"

void handle_io_process_syscall(uint32_t pid, uint32_t pc, uint32_t sleep_time, char *device_name); void handle_io_connection_not_found(uint32_t pid, uint32_t sleep_time, char *device_name);

#endif