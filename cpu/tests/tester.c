
#include "../utils.h"

int main(){

    int memory_server = create_server("30002");
    int dispatch_server = create_server("30003");
    int interrupt_server = create_server("30004");

    printf("Test Servers created.\n");

    int cpu_memory_connection;
    int cpu_dispatch_connection;
    int cpu_interrupt_connection;

    while ( cpu_dispatch_connection < 0 || cpu_memory_connection < 0 || cpu_interrupt_connection < 0 ) {
        cpu_memory_connection = accept_connection(memory_server);
        cpu_dispatch_connection = accept_connection(dispatch_server);
        cpu_interrupt_connection = accept_connection(interrupt_server);
        if (cpu_memory_connection < 0 || cpu_dispatch_connection < 0 || cpu_interrupt_connection < 0) {
            printf("Waiting for connections...\n");
            sleep(3);
        }
    }
    printf("Connections accepted.\n");

    t_buffer* buffer = buffer_create(sizeof(uint32_t) * 2);
    buffer_add_uint32(buffer, 1);
    buffer_add_uint32(buffer, 1);
    t_package* package = package_create(PID_PC_PACKAGE, buffer);
    printf("Package created.\n");

    printf("Press Enter to send package to CPU memory connection.\n");
    getchar();

    send_package(-20, package);
    printf("Package sent to CPU dispatch connection.\n");
    
    getchar();

    return 0;
}