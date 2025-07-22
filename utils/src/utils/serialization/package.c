#include "package.h"

t_package* create_package(OPCODE opcode, t_buffer* buffer) 
{
    t_package* package = safe_malloc(sizeof(t_package));
    package->opcode = opcode;
    package->buffer = buffer;
    return package;
}

void destroy_package(t_package* package) 
{
    if (package) {
        buffer_destroy(package->buffer);
        free(package);
        package = NULL;
    } else {
        LOG_WARNING("You tried to destroy NULL package");
    }
}

int send_package(int socket, t_package* package) 
{
    int32_t bytes;
    void* serialized_data = serialize_package(package, &bytes);
    int sent = send(socket, serialized_data, bytes, 0);
    free(serialized_data);
    return sent;
}


void* serialize_package(t_package* package, int32_t* total_size) 
{
    *total_size = sizeof(OPCODE) + sizeof(int32_t) + package->buffer->stream_size;

    void* stream = safe_malloc(*total_size); 

    int32_t offset = 0;

    memcpy(stream + offset, &(package->opcode), sizeof(OPCODE));
    offset += sizeof(OPCODE);

    memcpy(stream + offset, &(package->buffer->stream_size), sizeof(int32_t));
    offset += sizeof(int32_t);

    memcpy(stream + offset, package->buffer->stream, package->buffer->stream_size);

    return stream;
}

t_package* recv_package(int socket) 
{
    OPCODE opcode;
    int32_t buffer_stream_size;

    if (recv(socket, &opcode, sizeof(OPCODE), MSG_WAITALL) <= 0) {
        LOG_WARNING("socket %d disconnected", socket);
        return NULL; 
    }

    if (recv(socket, &buffer_stream_size, sizeof(int32_t), MSG_WAITALL) <= 0) {
        LOG_ERROR("Failed to receive buffer stream size from socket %d", socket);
        return NULL; 
    }

    void* buffer_stream_data = safe_malloc(buffer_stream_size);

    if (recv(socket, buffer_stream_data, buffer_stream_size, MSG_WAITALL) <= 0) {
        LOG_ERROR("Failed to receive buffer stream data from socket %d", socket);
        free(buffer_stream_data); 
        return NULL;
    }

    t_buffer* buffer = buffer_create(buffer_stream_size);
    buffer_add(buffer, buffer_stream_data, buffer_stream_size);

    t_package* package = create_package(opcode, buffer);

    free(buffer_stream_data);

    return package;
}