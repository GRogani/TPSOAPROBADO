#include "unsuspend-process.h"

void unsuspend_process_request_handler(int client_fd, t_package *package)
{
    uint32_t pid = read_swap_package(package);

    LOG_INFO("Solicitud de UNSUSPEND recibida para proceso PID: %u", pid);
    
    int result = swap_in_process(pid);
    
    if (result == 0) {
        LOG_INFO("UNSUSPEND exitoso para el proceso PID: %u", pid);
        send_confirmation_package(client_fd, 0); // 0 indicates success
    } else {
        LOG_ERROR("Error al realizar UNSUSPEND para el proceso PID: %u", pid);
        send_confirmation_package(client_fd, 1);
    }
}
