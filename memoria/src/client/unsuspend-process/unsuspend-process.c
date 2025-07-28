#include "unsuspend-process.h"

void unsuspend_process_request_handler(int client_fd, t_package *package)
{
    int32_t pid = read_swap_package(package);

    LOG_INFO("Solicitud de UNSUSPEND recibida para proceso PID: %u", pid);
    
    delay_swap_access();

    bool result = swap_in_process(pid);

    if (result) {
        LOG_INFO("UNSUSPEND exitoso para el proceso PID: %u", pid);
        send_confirmation_package(client_fd, true);
    } else {
        LOG_ERROR("Error al realizar UNSUSPEND para el proceso PID: %u", pid);
        send_confirmation_package(client_fd, false);
    }
}
