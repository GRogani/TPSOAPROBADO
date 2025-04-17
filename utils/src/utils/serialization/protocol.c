#include "protocol.h"

t_buffer *buffer_create(uint32_t size){
    // if (size == 0) 
    // {
    //     LOG_WARN("El tamaño del buffer a crear es cero.\n");
    //     return NULL;            
    // }

    t_buffer *buffer = safe_malloc(sizeof(t_buffer));

    buffer->offset = 0;
    buffer->size = size;

    if (size !=0)
        buffer->stream = safe_calloc(1 ,buffer->size);  // Ahi deje el calloc/ reserva 1 buffer inicializado en 0
    else buffer->stream = NULL;

    return buffer;
}

void buffer_destroy(t_buffer *buffer){
    if(buffer)
    {
        free(buffer->stream);
        free(buffer);
    }
    else 
    {
        LOG_WARN("Intentaste destruir un buffer NULL.\n");
    }
}

void buffer_add(t_buffer *buffer, void *data, uint32_t size){
    if (buffer->offset + size > buffer->size) {
        uint32_t new_size = buffer->offset + size;
        buffer->stream = safe_realloc(buffer->stream, new_size);
        buffer->size = new_size;
    }

    memcpy(buffer->stream + buffer->offset, data, size);
    buffer->offset += size;
}


void buffer_add_uint32(t_buffer *buffer, uint32_t data){
    buffer_add(buffer, &data, sizeof(uint32_t));
}

void buffer_add_string(t_buffer *buffer, uint32_t length, char *string){
    buffer_add_uint32(buffer, length);
    buffer_add(buffer, string, length);
}

void buffer_add_pointer(t_buffer *buffer, void *ptr) {
    uintptr_t ptr_as_integer = (uintptr_t)ptr; // Convertir el puntero a un entero sin signo
    buffer_add(buffer, &ptr_as_integer, sizeof(uintptr_t));
}


void buffer_read(t_buffer *buffer, void *data, uint32_t size){
   	if (buffer == NULL)
	{
        LOG_WARN("El buffer es NULL.\n");
		return;
	}
    if (data == NULL)
    {
        LOG_WARN("El puntero donde almacenar los datos es NULL.\n");
        return;
    }

	// Verificar límites de lectura
	if (buffer->offset + size > buffer->size)
	{
        LOG_WARN("Se quiere leer más de lo permitido ojito. Offset: %u, Size: %u, Buffer size: %u\n", buffer->offset, size, buffer->size);
		return;
	}

	memcpy(data, buffer->stream + buffer->offset, size);

	// Actualizar el desplazamiento del buffer
	buffer->offset += size;
}

uint32_t buffer_read_uint32(t_buffer *buffer){
    uint32_t data;
    buffer_read(buffer, &data, sizeof(uint32_t));
    return data;
}

char *buffer_read_string(t_buffer *buffer, uint32_t *length){
    *length = buffer_read_uint32(buffer);
    char *string = malloc(*length);
    buffer_read(buffer, string, *length);
    return string;
}

void* buffer_read_pointer(t_buffer *buffer) {
    uintptr_t ptr_as_integer;
    buffer_read(buffer, &ptr_as_integer, sizeof(uintptr_t)); // Leer el puntero como entero
    return (void*)ptr_as_integer; // Convertir de vuelta a un puntero
}

//Funciones para crear paquetes
t_package *package_create(cod_op cod_op, t_buffer *buffer){
    t_package *package = malloc(sizeof(t_package));
    package->codigo_operacion = cod_op;
    package->buffer = buffer;
    return package;
}

void *package_get_stream(t_package *package){
    void* to_send = malloc(sizeof(cod_op) + package->buffer->size + sizeof(uint32_t));
    int offset = 0;
    memcpy(to_send + offset, &package->codigo_operacion, sizeof(cod_op));
    offset += sizeof(cod_op);
    memcpy(to_send + offset, &package->buffer->size, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(to_send + offset, package->buffer->stream, package->buffer->size);
    return to_send;
}

void stream_destroy(void *stream){
    free(stream);
}

void package_destroy(t_package *package){
    buffer_destroy(package->buffer);
    free(package);
}
