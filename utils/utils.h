#ifndef UTILS_H
#define UTILS_H

// Modifiquen aca si cambian las utils

// Macros
#include "src/macros/logger_macro.h"

// Safe Alloc
#include "src/utils/safe_alloc.h"

// Enums
#include "src/enums/Eopcodes.h"
#include "src/enums/Eplanification_algorithm.h"
#include "src/enums/Einstruction_codes.h"

// Logger
#include "src/utils/logger/logger.h"

// Configs
#include "src/utils/config/config.h"
#include "src/utils/config/t_configs.h"

// Time Utils
#include "src/utils/time_utils.h"

// Serialization
#include "src/utils/serialization/buffer.h"
#include "src/utils/serialization/package.h"

// Socket
#include "src/utils/socket/client.h"
#include "src/utils/socket/server.h"

// Shutdown
#include "src/utils/shutdown.h"

// Enums
#include "src/enums/Eopcodes.h"
#include "src/enums/Eplanification_algorithm.h"
#include "src/enums/Eprocess_state.h"
#include "src/enums/Esyscall_type.h"

// DTPs
#include "src/utils/DTPs/confirmation_package.h"
#include "src/utils/DTPs/cpu_context_package.h"
#include "src/utils/DTPs/dispatch_package.h"
#include "src/utils/DTPs/dump_memory_package.h"
#include "src/utils/DTPs/fetch_package.h"
#include "src/utils/DTPs/init_process_package.h"
#include "src/utils/DTPs/instruction_package.h"
#include "src/utils/DTPs/interrupt_package.h"
#include "src/utils/DTPs/io_completion_package.h"
#include "src/utils/DTPs/io_operation_package.h"
#include "src/utils/DTPs/new_io_package.h"
#include "src/utils/DTPs/syscall_package.h"
#include "src/utils/DTPs/kill_process_package.h"
#include "src/utils/DTPs/swap_package.h"

#include "src/utils/DTPs/mmu_request_page_read_response.h"
#include "src/utils/DTPs/memory_write_request.h"
#include "src/utils/DTPs/memory_read_response.h"
#include "src/utils/DTPs/memory_read_request.h"
#include "src/utils/DTPs/page_entry_package.h"

#endif
