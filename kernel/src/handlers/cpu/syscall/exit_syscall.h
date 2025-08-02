#ifndef KERNEL_EXIT_SYSCALL_H
#define KERNEL_EXIT_SYSCALL_H

#include "../utils.h"
#include "repository/process/exit_list.h"
#include "kernel_logic/scheduler/long_scheduler.h"
#include "kernel_logic/scheduler/short_scheduler.h"
#include "repository/pcb/pcb.h"

void exit_process_syscall(int32_t syscall);

bool exit_routine(t_pcb *);

void log_process_metrics(int32_t pid, t_state_metrics state_metrics, t_time_metrics time_metrics);

void process_schedulers_exit();

#endif