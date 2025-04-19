#include "package.h"

t_package* package_create(OPCODE opcode, t_buffer* buffer) 
{
    t_package* package = safe_malloc(sizeof(t_package));
    package->opcode = opcode;
    package->buffer = buffer;
    return package;
}

void package_destroy(t_package* package) 
{
    if (package != NULL) {
        buffer_destroy(package->buffer);
        free(package);
    } else {
        LOG_WARN("You tried to destroy NULL package.");
    }
}

int send_package(int socket, t_package* package) 
{
    uint32_t bytes;
    void* serialized_data = package_serialize(package, &bytes);
    int sent = send(socket, serialized_data, bytes, 0);
    free(serialized_data);
    return sent;
}

// LEER IMPORTANTE
void* package_serialize(t_package* package, uint32_t* total_size) 
{
    
    //GUARDA EL SIZE DEL PACKETE SERIALIZADO EN ESA VARIABLE
    //OSEA QUE LE PASAS A LA FUNCION UN LUGAR PARA QUE LA GUARDE, DESPUES SE USA EN EL SEND
    *total_size = sizeof(OPCODE) + sizeof(uint32_t) + package->buffer->stream_size;

    void* stream = safe_malloc(*total_size); // El stream que va a viajar en el send

    uint32_t offset = 0;

    // Copiar opcode
    memcpy(stream + offset, &(package->opcode), sizeof(OPCODE));
    offset += sizeof(OPCODE);

    // Copiar tamaño del stream del buffer
    memcpy(stream + offset, &(package->buffer->stream_size), sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Copiar datos del buffer
    memcpy(stream + offset, package->buffer->stream, package->buffer->stream_size);

    return stream;
    // Osea como quedaria el stream

    //  (4 bytes)      (4 bytes)         (n bytes)      ... en realidad buffer->stream_size bytes
    // [  OPCODE  ] [ BUFFER_SIZE ] [BUFFER_STREAM_DATA]

    // Osea que no se manda el struct t_buffer, ver que no hay offset
    // Despues va a haber funciones read segun el opcode, asi que estaria valido esto

    //despues borro coments
}

t_package* recv_package(int socket) 
{
    OPCODE opcode;
    uint32_t buffer_stream_size;

    // recibir opcode
    // aca ya se puede validar si vale la pena seguir solo con el opcode
    if (recv(socket, &opcode, sizeof(OPCODE), MSG_WAITALL) <= 0) { 
        return NULL; 
    }

    // recibir el buffer size
    // quiza es muy grande se puede validar eso
    if (recv(socket, &buffer_stream_size, sizeof(uint32_t), MSG_WAITALL) <= 0) {
        return NULL; 
    }

    // hago espacio para buffer serializado
    void* buffer_stream_data = safe_malloc(buffer_stream_size);

    // guardo el buffer stream de data serializado / caso error libero memoria
    if (recv(socket, buffer_stream_data, buffer_stream_size, MSG_WAITALL) <= 0) {
        free(buffer_stream_data); 
        return NULL;
    }

    t_buffer* buffer = buffer_create(buffer_stream_size); // Crea un buffer con el tamaño recibido
    buffer_add(buffer, buffer_stream_data, buffer_stream_size); // Agrega los datos serializados al buffer

    buffer->offset = 0;

    t_package* package = package_create(opcode, buffer);

    free(buffer_stream_data); // liberar memoria de los datos serializados -> esto no te elimina la info que devolves?

    return package;
}

