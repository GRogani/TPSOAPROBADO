#include "main.h"

int main(int argc, char* argv[]) 
{
    t_config* config_cpu = init_config("cpu.config");

    t_log_level level = log_level_from_string(config_get_string_value(config_cpu,"LOG_LEVEL"));
    init_logger("cpu.log", "CPU", level);

    int fd_memory = create_connection(config_get_string_value(config_cpu, "PUERTO_MEMORIA"), config_get_string_value(config_cpu, "IP_MEMORIA"));
    int fd_kernel_dispatch = create_connection(config_get_string_value(config_cpu, "PUERTO_KERNEL_DISPATCH"), config_get_string_value(config_cpu, "IP_KERNEL_DISPATCH"));
    int fd_kernel_interrupt = create_connection(config_get_string_value(config_cpu, "PUERTO_KERNEL_INTERRUPT"), config_get_string_value(config_cpu, "IP_KERNEL_INTERRUPT"));

    int c;
    do
    {
        c = getchar();
    } while (c != '\n' && c != EOF);

    char* yourName = "Hello world";

    uint32_t len = strlen(yourName) + 1;
    t_buffer *buffer = buffer_create(len);
    buffer_add_string(buffer, len, yourName);
    t_package* package = package_create(HANDSHAKE, buffer);

    int bytes_sent = send_package(fd_kernel_dispatch, package);
    package_destroy(package);

    printf("Sent package 1\n");

    int d;
    do
    {
        d = getchar();
    } while (d != '\n' && d != EOF);

    char *yourName2 = "Hello world";

    uint32_t len2 = strlen(yourName2) + 1;
    t_buffer *buffer2 = buffer_create(len);
    buffer_add_string(buffer2, len2, yourName2);
    t_package *package2 = package_create(HANDSHAKE, buffer2);

    send_package(fd_kernel_dispatch, package2);
    package_destroy(package2);

    printf("Sent package 2\n");

    sleep(1111111111111111111);

    // close(fd_memory);
    close(fd_kernel_dispatch);
    close(fd_kernel_interrupt);
    config_destroy(config_cpu);
    return 0;
}
