#include "protocolo_test.h"

context (protocolo_test) {

    describe("Buffer Creation") {

        t_buffer* buffer;

        before {
            buffer = NULL;
        } end

        after {
            buffer_destroy(buffer);
        } end

        it("should create a buffer with correct initial values") {
            buffer = buffer_create(128);

            should_ptr(buffer) not be null;
            should_int(buffer->size) be equal to(128);
            should_int(buffer->offset) be equal to(0);
            should_ptr(buffer->stream) not be null;

        } end

        it("should return NULL when creating buffer of size 0") {
            buffer = buffer_create(0);
            should_ptr(buffer) be null;
        } end

    } end

    describe("Buffer Addition of Data") {

        t_buffer* buffer;

        before {
            buffer = NULL;
        } end

        after {
            buffer_destroy(buffer);
        } end

        it("should add and read a uint32 correctly") {
            buffer = buffer_create(64);
            uint32_t original = 42;

            buffer_add_uint32(buffer, original);

            buffer->offset = 0; // Reset offset para leer
            uint32_t result = buffer_read_uint32(buffer);
            should_int(result) be equal to(original); // Usar la variable original para la comparación

            
        } end

        it("should add and read a string correctly") {
            buffer = buffer_create(64);
            char* original_str = "Hola mundo";
            uint32_t len = strlen(original_str) + 1;

            buffer_add_string(buffer, len, (char*)original_str);

            buffer->offset = 0;
            uint32_t read_len;
            char* result = buffer_read_string(buffer, &read_len);

            should_int(read_len) be equal to(len);
            should_string(result) be equal to(original_str);

            free(result);
            
        } end

        it("should add and read a pointer correctly") {
            buffer = buffer_create(64);
            int value = 123;
            buffer_add_pointer(buffer, &value);

            buffer->offset = 0;
            int* result = (int*) buffer_read_pointer(buffer);

            should_int(*result) be equal to(123);

            
        } end

        it("should handle NULL pointer safely when adding") {
            buffer = buffer_create(64);

            buffer_add_pointer(buffer, NULL); // Añadir puntero NULL
            buffer->offset = 0;
            void* result = buffer_read_pointer(buffer);

            should_ptr(result) be null; // Debe retornar NULL si se añade un puntero NULL

            
        } end

    }end

    describe("Paquete Structure") {

        t_buffer* buffer;

        before {
            buffer = NULL;
        } end

        after {
            buffer_destroy(buffer);
        } end

        it("should correctly create a paquete with HANDSHAKE and buffer") {
            buffer = buffer_create(32);
            uint32_t original = 2025;
            buffer_add_uint32(buffer, original);

            t_paquete paquete;
            paquete.codigo_operacion = HANDSHAKE;
            paquete.buffer = buffer;

            should_int(paquete.codigo_operacion) be equal to(HANDSHAKE);
            should_ptr(paquete.buffer) not be null;
            should_int(paquete.buffer->size) be equal to(32);

            buffer->offset = 0;
            uint32_t result = buffer_read_uint32(paquete.buffer);
            should_int(result) be equal to(original);

        } end

        it("should handle NULL buffer in paquete") {
            t_paquete paquete;
            paquete.codigo_operacion = HANDSHAKE;
            paquete.buffer = NULL;

            should_int(paquete.codigo_operacion) be equal to(HANDSHAKE);
            should_ptr(paquete.buffer) be null;
        } end

    } end

    describe("Buffer Boundary Conditions") {

        t_buffer* buffer;

        before {
            buffer = NULL;
        } end

        after {
            buffer_destroy(buffer);
        } end

        it("should prevent buffer overflow on write") {
            buffer = buffer_create(4);
            uint32_t big_data[2] = {1, 2};

            buffer_add(buffer, big_data, sizeof(big_data)); // Debería manejar el overflow internamente

            // Verificar que el buffer no se ha desbordado
            should_int(buffer->offset) be equal to(0); // El offset no debe haber cambiado

            
        } end

        it("should prevent buffer underflow on read") {
             buffer = buffer_create(4);
            uint32_t data = 42;
            buffer_add_uint32(buffer, data);

            uint64_t big_read;
            buffer_read(buffer, &big_read, sizeof(big_read)); // Debería manejar el underflow internamente

            
        } end 
    } end
}
