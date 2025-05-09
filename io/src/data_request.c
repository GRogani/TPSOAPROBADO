#include "data_request.h"
#include "utils/logger/logger.h"

void request_destroy(t_request_IO* request) {
    if (request) {
        free(request);
    } else {
        LOG_WARN("You tried to destroy NULL t_request_IO.");
    }
}