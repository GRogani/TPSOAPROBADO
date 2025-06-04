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

// DTOs
#include "src/utils/DTOs/cpu_dispatch.h"
#include "src/utils/DTOs/cpu_interrupt.h"
#include "src/utils/DTOs/cpu_syscall.h"
#include "src/utils/DTOs/io_new_device.h"
#include "src/utils/DTOs/io_operation_completed.h"
#include "src/utils/DTOs/io_operation_request.h"
#include "src/utils/DTOs/memory_create_process.h"
#include "src/utils/DTOs/memory_get_instruction.h"

#endif 
