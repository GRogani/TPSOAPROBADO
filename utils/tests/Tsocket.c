#include "Tsocket.h"

#define TEST_PORT "42069"
#define TEST_MESSAGE "Tralalero Tralala"
#define TEST_MESSAGE_LEN 18

void* test_server_recv_package(void* _) {
    int server_socket = create_server(TEST_PORT);
    if (server_socket == -1) return NULL;

    int client_socket = accept_connection(server_socket);
    if (client_socket == -1) return NULL;

    t_package* received = recv_package(client_socket);
    if (!received) return NULL;

    should_ptr(received) not be equal to(NULL);
    should_int(received->opcode) be equal to(HANDSHAKE);

    char* message = read_handshake(received);

    should_int(strlen(message) + 1) be equal to(TEST_MESSAGE_LEN);
    should_string(message) be equal to(TEST_MESSAGE);

    free(message);
    package_destroy(received);
    close(client_socket);
    close(server_socket);
    return NULL;
}


void* test_server_send_package(void* _) {
    int server_socket = create_server(TEST_PORT);
    if (server_socket == -1) {
        return NULL;
    }

    int client_socket = accept_connection(server_socket);
    if (client_socket == -1) {
        return NULL;
    }

    send_

    package_destroy(pkg);
    close(client_socket);
    close(server_socket);
    return NULL;
}

context(socket_package_tests)
{
    describe("Send and receive t_package between sockets")
    {
        before {
            init_logger("test.log", "[TESTS]", LOG_LEVEL_ERROR);
        } end

        after {
            destroy_logger();
        } end

        it("should send a serialized package from client and receive on server")
        {
            pthread_t server_thread;
            pthread_create(&server_thread, NULL, test_server_recv_package, NULL);

            sleep(1); // esperar que el server este escuchando

            int client_socket = create_connection(TEST_PORT, "127.0.0.1");
            should_int(client_socket) not be equal to(-1);

            t_buffer* buffer = buffer_create(TEST_MESSAGE_LEN);
            buffer_add_string(buffer, TEST_MESSAGE_LEN, TEST_MESSAGE);

            t_package* pkg = package_create(HANDSHAKE, buffer);

            // Podes imprimir con read_handshake antes de enviar
            char* msg = read_handshake(pkg);
            free(msg);

            int bytes_sent = send_package(client_socket, pkg);
            should_bool(bytes_sent > 0) be equal to(true);

            package_destroy(pkg);
            close(client_socket);
            pthread_join(server_thread, NULL);
        } end



        it("should receive a serialized package sent by server")
        {
            pthread_t server_thread;
            pthread_create(&server_thread, NULL, test_server_send_package, NULL);

            sleep(1);

            int client_socket = create_connection(TEST_PORT, "127.0.0.1");
            should_int(client_socket) not be equal to(-1);

            t_package* received = recv_package(client_socket);
            should_ptr(received) not be equal to(NULL);
            should_int(received->opcode) be equal to(HANDSHAKE);

            char* mensaje = read_handshake(received);
            should_int(strlen(mensaje) + 1) be equal to(TEST_MESSAGE_LEN);
            should_string(mensaje) be equal to(TEST_MESSAGE);

            free(mensaje);
            package_destroy(received);
            close(client_socket);
            pthread_join(server_thread, NULL);
        } end

    } end
}
