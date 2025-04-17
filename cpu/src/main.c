#include <unistd.h>
#include <utils/config/config.h>
#include <utils/socket/client.h>
#include <utils/logger/logger.h>
#include <commons/string.h>
#include <utils/serialization/protocol.h>

int main(int argc, char* argv[]) {
t_config* config_cpu = init_config("cpu.config");

t_log_level level = log_level_from_string(config_get_string_value(config_cpu,"LOG_LEVEL"));
init_logger("cpu.log", "CPU", level);

int fd_memory = create_connection(config_get_string_value(config_cpu, "PUERTO_MEMORIA"), config_get_string_value(config_cpu, "IP_MEMORIA"));
int fd_kernel_dispatch = create_connection(config_get_string_value(config_cpu, "PUERTO_KERNEL_DISPATCH"), config_get_string_value(config_cpu, "IP_KERNEL_DISPATCH"));
int fd_kernel_interrupt = create_connection(config_get_string_value(config_cpu, "PUERTO_KERNEL_INTERRUPT"), config_get_string_value(config_cpu, "IP_KERNEL_INTERRUPT"));

t_buffer* buffer = buffer_create(sizeof(uint32_t) + string_length("Hola desde cpu") + 1);
buffer_add_uint32(buffer, string_length("Hola desde cpu"));
buffer_add_string(buffer, string_length("Hola desde cpu"), "Hola desde cpu");
t_package* package = package_create(HANDSHAKE, buffer);
void* stream = package_get_stream(package);
send(fd_memory, stream, sizeof(OPCODE) + buffer->size + sizeof(uint32_t), 0);
send(fd_kernel_dispatch, stream, sizeof(OPCODE) + buffer->size + sizeof(uint32_t), 0);

free(stream);
package_destroy(package);


close(fd_memory);
close(fd_kernel_dispatch);
close(fd_kernel_interrupt);
config_destroy(config_cpu);
return 0;
}
