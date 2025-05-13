#include "handshake.h"

void process_handshake(t_package* package)
{
    char* id = read_handshake(package);
    if (id == NULL) {
        log_error(get_logger(), "Failed to read handshake");
        package_destroy(package);
        return;
    }
    
    log_info(get_logger(), "Handshake received from : %s", id);
    free(id);
    package_destroy(package);
}