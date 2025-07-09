#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "../utils.h"

#define MEMORIA_PORT "80011" // Change if needed
#define MEMORIA_IP   "127.0.0.1"
#define TEST_PID     4242
#define TEST_SIZE    4096

t_memoria_config memoria_config;

int main() {
    t_config *config_file = init_config("memoria.config");
    memoria_config = init_memoria_config(config_file);
    init_logger("test.log", "TEST", memoria_config.LOG_LEVEL);

    LOG_INFO("Connecting to memoria at %s:%s...", MEMORIA_IP, MEMORIA_PORT);
    int sock = create_connection(MEMORIA_PORT, MEMORIA_IP);
    if (sock <= 0) {
        LOG_ERROR("Failed to connect to memoria!");
        return 1;
    }
    LOG_INFO("Connected!");

    // 1. Send INIT_PROCESS
    LOG_INFO("Sending INIT_PROCESS for PID %d, size %d...", TEST_PID, TEST_SIZE);
    t_package* init_pkg = create_init_process_package(TEST_PID, TEST_SIZE, "");
    init_pkg->opcode = INIT_PROCESS;
    int sent = send_package(sock, init_pkg);
    if (sent <= 0) {
        LOG_ERROR("Failed to send INIT_PROCESS!");
        destroy_package(init_pkg);
        close(sock);
        return 1;
    }
    destroy_package(init_pkg);
    LOG_INFO("INIT_PROCESS sent.");
    sleep(1); // Give memoria time to process

    // 2. Send UNSUSPEND_PROCESS
    LOG_INFO("Sending UNSUSPEND_PROCESS for PID %d...", TEST_PID);
    t_package* unsuspend_pkg = create_swap_package(TEST_PID);
    unsuspend_pkg->opcode = UNSUSPEND_PROCESS;
    sent = send_package(sock, unsuspend_pkg);
    if (sent <= 0) {
        LOG_ERROR("Failed to send UNSUSPEND_PROCESS!");
        destroy_package(unsuspend_pkg);
        close(sock);
        return 1;
    }
    destroy_package(unsuspend_pkg);
    LOG_INFO("UNSUSPEND_PROCESS sent.");
    sleep(1);

    // 3. Optionally, send KILL_PROCESS to clean up
    LOG_INFO("Sending KILL_PROCESS for PID %d...", TEST_PID);
    t_package* kill_pkg = create_kill_process_package(TEST_PID);
    kill_pkg->opcode = KILL_PROCESS;
    sent = send_package(sock, kill_pkg);
    if (sent <= 0) {
        LOG_ERROR("Failed to send KILL_PROCESS!");
        destroy_package(kill_pkg);
        close(sock);
        return 1;
    }
    destroy_package(kill_pkg);
    LOG_INFO("KILL_PROCESS sent.");

    // 4. Optionally, wait for a response (if memoria sends one)
    // t_package* response = recv_package(sock);
    // if (response) { LOG_INFO("Got response from memoria!"); destroy_package(response); }

    close(sock);
    LOG_INFO("Integration test completed. Check memoria logs for details.");
    return 0;
}