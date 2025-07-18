#ifndef MAIN_H
#define MAIN_H

#include <signal.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/log.h>
#include "../utils.h"
#include "semaphores.h"
#include "client/handlers.h"
#include "kernel_space/page_table.h"
#include "user_space/frame_manager.h"
#include "swap_space/swap_manager.h"

#endif