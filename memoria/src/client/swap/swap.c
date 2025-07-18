#include "swap.h"

void swap_request_handler(int client_fd, t_package *package)
{
    uint32_t pid = read_swap_package(package);

    LOG_INFO("Solicitud de SWAP recibida para proceso PID: %u", pid);
    
    int result = swap_out_process(pid);
    
    delay_swap_access();

    if (result == 0) {
        LOG_INFO("SWAP exitoso para el proceso PID: %u", pid);
        send_confirmation_package(client_fd, 0); // 0 indicates success
    } else {
        LOG_ERROR("Error al realizar SWAP para el proceso PID: %u", pid);
        send_confirmation_package(client_fd, 1);
    }
}
