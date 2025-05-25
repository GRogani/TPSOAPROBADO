#include "Tbuffer.h"

context (ProtocolUtils) {

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
            should_int(buffer->stream_size) be equal to(128);
            should_int(buffer->offset) be equal to(0);
            should_ptr(buffer->stream) not be null;
        } end

        it("should handle very large buffer creation") {
            buffer = buffer_create(1 << 24); // 16MB buffer
            should_ptr(buffer) not be null;
            should_int(buffer->stream_size) be equal to(1 << 24);
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

            should_int(buffer->offset) be equal to(sizeof(uint32_t));
            
            buffer->offset = 0; // Reset offset to read
            
            uint32_t result = buffer_read_uint32(buffer);
            should_int(result) be equal to(original);
            should_int(buffer->offset) be equal to(sizeof(uint32_t));
        } end

        it("should add and read multiple uint32 sequentially") {
            buffer = buffer_create(128);
            uint32_t values[] = {1, 2, 3, 4, 5};
            
            for (int i = 0; i < 5; i++) {
                buffer_add_uint32(buffer, values[i]);
            }
            
            buffer->offset = 0;
            for (int i = 0; i < 5; i++) {
                uint32_t result = buffer_read_uint32(buffer);
                should_int(result) be equal to(values[i]);
                should_int(buffer->offset) be equal to((i+1) * sizeof(uint32_t));
            }
        } end

        it("should add and read a string correctly (including null terminator)") {
            buffer = buffer_create(64);
            char original_str[] = "Hola mundo";
            uint32_t len = strlen(original_str) + 1;  // Including '\0'

            buffer_add_string(buffer, len, original_str);

            should_int(buffer->offset) be equal to(len + sizeof(uint32_t)); // Should include length + uint32 header
            
            buffer->offset = 0;
            uint32_t read_len;
            char* result = buffer_read_string(buffer, &read_len);

            should_int(read_len) be equal to(len);  // Should include '\0'
            should_string(result) be equal to(original_str);
            should_int(buffer->offset) be equal to(len + sizeof(uint32_t));  // Offset should have moved correctly

            free(result);
        } end

        it("should add and read an empty string correctly (including null terminator)") {
            buffer = buffer_create(64);
            char empty_str[] = "";
            uint32_t len = strlen(empty_str) + 1;  // Including '\0'

            buffer_add_string(buffer, len, empty_str);
            
            buffer->offset = 0;
            uint32_t read_len;
            char* result = buffer_read_string(buffer, &read_len);

            should_int(read_len) be equal to(len);  // Should include '\0'
            should_string(result) be equal to(empty_str);
            
            free(result);
        } end

        it("should add and read a pointer correctly") {
            buffer = buffer_create(64);
            int value = 123;
            buffer_add_pointer(buffer, &value);

            should_int(buffer->offset) be equal to(sizeof(void*));
            
            buffer->offset = 0;
            int* result = (int*) buffer_read_pointer(buffer);

            should_int(*result) be equal to(123);
            should_int(buffer->offset) be equal to(sizeof(void*));
        } end

        it("should handle NULL pointer safely when adding") {
            buffer = buffer_create(64);

            buffer_add_pointer(buffer, NULL);
            should_int(buffer->offset) be equal to(sizeof(void*));
            
            buffer->offset = 0;
            void* result = buffer_read_pointer(buffer);

            should_ptr(result) be null;
            should_int(buffer->offset) be equal to(sizeof(void*));
        } end

        it("should handle mixed data types correctly") {
            buffer = buffer_create(128);
            
            // Add different types
            uint32_t num = 42;
            char str[] = "test";
            uint32_t str_len = strlen(str) + 1;  // Including '\0'
            int value = 123;
            
            buffer_add_uint32(buffer, num);
            buffer_add_string(buffer, str_len, str);
            buffer_add_pointer(buffer, &value);
            
            // Verify offsets after each addition
            should_int(buffer->offset) be equal to(
                sizeof(uint32_t) +      // num
                sizeof(uint32_t) +      // str_len
                str_len +               // str
                sizeof(void*)          // pointer
            );
            
            // Read back and verify
            buffer->offset = 0;
            
            uint32_t read_num = buffer_read_uint32(buffer);
            should_int(read_num) be equal to(num);
            
            uint32_t read_len;
            char* read_str = buffer_read_string(buffer, &read_len);
            should_string(read_str) be equal to(str);
            free(read_str);
            
            int* read_ptr = (int*) buffer_read_pointer(buffer);
            should_int(*read_ptr) be equal to(value);
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

            buffer_add(buffer, big_data, sizeof(big_data));

            // Verify data was written correctly
            uint32_t result[2] = {0};
            buffer->offset = 0;
            buffer_read(buffer, result, sizeof(result));

            should_int(result[0]) be equal to(1);
            should_int(result[1]) be equal to(2);
            should_int(buffer->offset) be equal to(sizeof(big_data));
        } end

        it("should prevent buffer underflow on read") {
            buffer = buffer_create(4);
            uint32_t data = 42;
            buffer_add_uint32(buffer, data);

            uint64_t big_read = 0xDEADBEEF;
            buffer_read(buffer, &big_read, sizeof(big_read));

            // Verify value didn't change
            should_long(big_read) be equal to(0xDEADBEEF);
            should_int(buffer->offset) be equal to(sizeof(uint32_t)); // Should only advance by what was available
        } end

        it("should handle reading/writing at non-zero offsets") {
            buffer = buffer_create(128);
            
            // Write some initial data
            uint32_t dummy = 999;
            buffer_add_uint32(buffer, dummy);
            
            // Store current offset
            size_t initial_offset = buffer->offset;
            
            // Add real data
            uint32_t num = 42;
            buffer_add_uint32(buffer, num);
            
            // Read from the middle
            buffer->offset = initial_offset;
            uint32_t result = buffer_read_uint32(buffer);
            
            should_int(result) be equal to(num);
            should_int(buffer->offset) be equal to(initial_offset + sizeof(uint32_t));
        } end

    } end

    describe("Buffer Utility Functions") {

        t_buffer* buffer;

        before {
            buffer = NULL;
        } end

        after {
            buffer_destroy(buffer);
        } end

        it("should correctly resize buffer when needed") {
            buffer = buffer_create(4);
            should_int(buffer->stream_size) be equal to(4);
            
            // Add more data than initial size
            uint32_t data[4] = {1, 2, 3, 4};
            buffer_add(buffer, data, sizeof(data));
            should_int(buffer->stream_size) be equal to(sizeof(data));  // Should resize the buffer
            buffer->offset = 0;
            
            uint32_t result[4] = {0};
            buffer_read(buffer, result, sizeof(result));
            
            for (int i = 0; i < 4; i++) {
                should_int(result[i]) be equal to(i+1);
            }
        } end

    } end

}
