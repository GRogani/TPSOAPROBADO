#include "../utils.h"

int main()
{
    printf("\t[TEST MODULO IO]\nEl ejecutable simula ser el kernel,\nse conecta al modulo IO y le envia dos paquetes de solicitud de IO al mismo tiempo.\n");
    
    t_buffer*   buffer1 = NULL;
    t_buffer*   buffer2 = NULL;
    t_package*  package1 = NULL;
    t_package*  package2 = NULL;
    t_package*  response = NULL;
    
    int kernel_server = create_server("30002");

    init_logger("tester.log", "[TESTER]", LOG_LEVEL_DEBUG);

    int socket = -1;
    while(socket < 0)
    {
        socket = accept_connection(kernel_server);
    }

    LOG_INFO("Connected to kernel server on socket %d", socket);

    response = recv_package(socket);
    LOG_INFO("Received package with opcode: %s", opcode_to_string(response->opcode) );
    package_destroy(response);

    printf("Press enter to send packages...\n");
    getchar();

    buffer1 = buffer_create(sizeof(uint32_t) * 2);
    buffer_add_uint32(buffer1, 1);
    buffer_add_uint32(buffer1, 10000);

    package1 = package_create(REQUEST_IO, buffer1);

    buffer2 = buffer_create(sizeof(uint32_t) * 2);
    buffer_add_uint32(buffer2, 2);
    buffer_add_uint32(buffer2, 20000);

    package2 = package_create(REQUEST_IO, buffer2);

    LOG_INFO("Sending 2 packages with opcode: %s", opcode_to_string(package1->opcode) );

    send_package(socket, package1);
    send_package(socket, package2);
    package_destroy(package1);
    package_destroy(package2);


    response = recv_package(socket);
    LOG_INFO("Received package with opcode: %s",  opcode_to_string(response->opcode) );
    package_destroy(response);

    printf("Press enter to exit...\n");
    getchar();

    return 0;
}