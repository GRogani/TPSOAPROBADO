#ifndef KERNEL_IO_SYSCALL_H
#define KERNEL_IO_SYSCALL_H

#include "../utils.h"
#include "repository/process/exit_list.h"
#include "kernel_logic/scheduler/long_scheduler.h"
#include "kernel_logic/scheduler/short_scheduler.h"
#include "kernel_logic/scheduler/medium_scheduler.h"
#include "repository/pcb/pcb.h"
#include "exit_syscall.h"
#include "../../io/process_pending_io.h"

void handle_io_process_syscall(int32_t pid, int32_t pc, int32_t sleep_time, char *device_name); void handle_io_connection_not_found(int32_t pid, int32_t sleep_time, char *device_name);

void* medium_scheduler_thread(void* arg); // wrapper

void process_schedulers_io();

void short_scheduler_thread();

#endif