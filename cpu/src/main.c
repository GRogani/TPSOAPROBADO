#include "main.h"
#include <utils/serialization/send.h>

typedef struct HandshakeDto {
    char* device_name;
    char* device_required_space;
} HandshakeDto;

int main(int argc, char* argv[]) 
{
    t_config* config_cpu = init_config("cpu.config");

    t_log_level level = log_level_from_string(config_get_string_value(config_cpu,"LOG_LEVEL"));
    init_logger("cpu.log", "CPU", level);

    int fd_memory = create_connection(config_get_string_value(config_cpu, "PUERTO_MEMORIA"), config_get_string_value(config_cpu, "IP_MEMORIA"));
    int fd_kernel_dispatch = create_connection(config_get_string_value(config_cpu, "PUERTO_KERNEL_DISPATCH"), config_get_string_value(config_cpu, "IP_KERNEL_DISPATCH"));
    int fd_kernel_interrupt = create_connection(config_get_string_value(config_cpu, "PUERTO_KERNEL_INTERRUPT"), config_get_string_value(config_cpu, "IP_KERNEL_INTERRUPT"));

    sendMessageToKernel(fd_kernel_dispatch);

    close(fd_memory);
    close(fd_kernel_dispatch);
    close(fd_kernel_interrupt);
    config_destroy(config_cpu);
    return 0;
}

void sendMessageToKernel(int fd_kernel_dispatch) {
    HandshakeDto* handshakeDto = malloc(sizeof(HandshakeDto));

    char* device_name = malloc(sizeof(char) * 30);
    strcpy(device_name, "Device 1");
    
    char* device_required_space = malloc(sizeof(char) * 30);
    strcpy(device_required_space, "1000");
    
    handshakeDto->device_name = device_name;
    handshakeDto->device_required_space = device_required_space;
    
    int messageResult = sendMessageFromStruct(fd_kernel_dispatch, OTHER, handshakeDto, 2);
    
    printf("Result of socket: %d", messageResult);
    
    free(handshakeDto->device_name);
    free(handshakeDto->device_required_space);
    free(handshakeDto);
}