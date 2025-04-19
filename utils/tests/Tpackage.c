#include "Tpackage.h"

context(PackageTests) {

    describe("Package creation and destruction") {

        it("should create a package with correct values") {
            t_buffer* buffer = buffer_create(64);
            t_package* package = package_create(HANDSHAKE , buffer);

            should_ptr(package) not be null;
            should_int(package->opcode) be equal to(HANDSHAKE);
            should_ptr(package->buffer) be equal to(buffer);

            package_destroy(package); // tambien destruye buffer
        } end

        it("should destroy NULL package safely") {
            // Deberia loggear warning pero no crashear
            package_destroy(NULL);
        } end
    } end

    describe("Package serialization") {
        it("should serialize correctly") {
            t_buffer* buffer = buffer_create(64);
            uint32_t value = 12345;
            buffer_add_uint32(buffer, value);

            t_package* package = package_create(42, buffer);

            uint32_t serialized_size;
            void* serialized = package_serialize(package, &serialized_size);

            should_ptr(serialized) not be null;
            should_int(serialized_size) be equal to(
                sizeof(OPCODE) + sizeof(uint32_t) + buffer->stream_size
            );

            // Verificamos el contenido del stream
            uint32_t offset = 0;
            OPCODE op;
            memcpy(&op, serialized + offset, sizeof(OPCODE));
            should_int(op) be equal to(42);
            offset += sizeof(OPCODE);

            uint32_t stream_size;
            memcpy(&stream_size, serialized + offset, sizeof(uint32_t));
            should_int(stream_size) be equal to(buffer->stream_size);
            offset += sizeof(uint32_t);

            uint32_t received_value;
            memcpy(&received_value, serialized + offset, sizeof(uint32_t));
            should_int(received_value) be equal to(value);

            free(serialized);
            package_destroy(package);
        } end
    } end

    describe("Package send and receive via socket") {

        it("should send and receive package successfully") {
            int fds[2];
            socketpair(AF_UNIX, SOCK_STREAM, 0, fds); // fds[0] <-> fds[1] esta bueno para testear esto

            t_buffer* buffer = buffer_create(32);
            uint32_t value = 789;
            buffer_add_uint32(buffer, value);

            t_package* sent_pkg = package_create(10, buffer);
            int sent_bytes = send_package(fds[0], sent_pkg);
            should_bool(sent_bytes>0) be equal to(true);

            t_package* received_pkg = recv_package(fds[1]);
            should_ptr(received_pkg) not be null;
            should_int(received_pkg->opcode) be equal to(10);

            received_pkg->buffer->offset = 0;
            uint32_t received_value = buffer_read_uint32(received_pkg->buffer);
            should_int(received_value) be equal to(value);

            package_destroy(sent_pkg);
            package_destroy(received_pkg);

            close(fds[0]);
            close(fds[1]);
        } end

        it("should return NULL on socket close during recv") {
            int fds[2];
            socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

            close(fds[0]); // cerramos un extremo para provocar error

            t_package* result = recv_package(fds[1]);
            should_ptr(result) be null;

            close(fds[1]);
        } end
    } end
}
