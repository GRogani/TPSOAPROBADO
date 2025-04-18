#include "Tsocket.h"

void* test_server1(void* _) {
    int server_socket = create_server(TEST_PORT);
    int client_socket = accept_connection(server_socket);

    close(client_socket);
    close(server_socket);
    return NULL;
}


void* test_server2(void* _) {
    int server_socket = create_server(TEST_PORT);
    int client_socket = accept_connection(server_socket);

    char buffer[64];
    recv(client_socket, buffer, sizeof(buffer), 0);
    log_info(get_logger(), "Received: %s", buffer);

    close(client_socket);
    close(server_socket);
    return NULL;
}

void* test_server3(void* _) {
    int server_socket = create_server(TEST_PORT);
    int client_socket1 = accept_connection(server_socket);
    int client_socket2 = accept_connection(server_socket);
    close(client_socket1);
    close(client_socket2);
    close(server_socket);
    return NULL;
}

context(socket_tests)
{
    describe("1 Client x 1 Server connection.")
    {
        before {
            init_logger("test.log", "[TESTS]", LOG_LEVEL_INFO);
        } end

        after {
            destroy_logger();
        } end

        it("should create 1 client socket and connect to test server")
        {
            pthread_t server_thread;
            pthread_create(&server_thread, NULL, test_server1, NULL);
            
            sleep(.5);
            int client_socket = create_connection(TEST_PORT, "127.0.0.1");

            sleep(1);
            int client_socket2 = create_connection(TEST_PORT, "127.0.0.1"); // esta la rechaza o tendria que

            should_int(client_socket) not be equal to(-1);
            should_int(client_socket2) be equal to(-1);

            close(client_socket);
            close(client_socket2);

            pthread_join(server_thread, NULL);

        } end

        it("should reject the connection if no server is up")
        {
            int client_socket = create_connection(TEST_PORT, "127.0.0.1");

            should_int(client_socket) be equal to(-1);

            close(client_socket);
        } end

        it("should handle send and receive calls") {
            pthread_t server_thread;
            pthread_create(&server_thread, NULL, test_server2, NULL);

            sleep(.5);

            int client_socket = create_connection(TEST_PORT, "127.0.0.1");

            should_int(client_socket) not be equal to(-1);

            char* message = "Tralalero Tralala";

            sleep(.5);
            int bytes_sent = send(client_socket, message, strlen(message) + 1, 0);

            should_bool(bytes_sent > 0) be equal to(true);

            close(client_socket);
            pthread_join(server_thread, NULL);
        } end


    } end
    
    // Este anda raro jajajaja despues lo veo bien
    // describe("N Client x 1 Server connection.")
    // {
    //     before {
    //         init_logger("test.log", "[TESTS]", LOG_LEVEL_INFO);
    //     } end

    //     after {
    //         destroy_logger();
    //     } end

    //     it("should create N client sockets and connect to test server")
    //     {
    //         pthread_t server_thread;
    //         pthread_create(&server_thread, NULL, test_server3, NULL);
            
    //         sleep(.5);

    //         int client_socket1 = create_connection(TEST_PORT, "127.0.0.1");
    //         int client_socket2 = create_connection(TEST_PORT, "127.0.0.1");

    //         should_int(client_socket1) not be equal to(-1);
    //         should_int(client_socket2) not be equal to(-1);
    //         should_int(client_socket1) not be equal to(client_socket2);

    //         close(client_socket1);
    //         close(client_socket2);
    //         pthread_join(server_thread, NULL);
    //     } end

    // } end
}