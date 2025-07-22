#ifndef KERNEL_TEST_SCHEDULERS_H
#define KERNEL_TEST_SCHEDULERS_H

#include "../utils.h"
#include <assert.h>
#include "repository/pcb/pcb.h"
#include "globals.h"

// memory_requests: contador de requests
void* memory_server_mock(void* memory_requests);
void start_memory_mock_server();

t_pcb* create_test_pcb(int32_t pid, int32_t size, const char* filename);

#endif