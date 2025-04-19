#ifndef UTILS_H
#define UTILS_H

// Modifiquen aca si cambian las utils

// Macros
#include "src/macros/log_error.h"

// Safe Alloc
#include "src/utils/safe_alloc.h"

// Logger
#include "src/utils/logger/logger.h"

// Configs
#include "src/utils/config/config.h"
#include "src/utils/config/t_configs.h"

// DTOs
#include "src/utils/DTOs/dtos.h"

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

#endif 
