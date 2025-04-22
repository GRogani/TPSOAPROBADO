#include "buffer.h"

t_buffer *buffer_create(uint32_t size){

    t_buffer *buffer = safe_malloc(sizeof(t_buffer));

    buffer->offset = 0;
    buffer->stream_size = size;

    if (size !=0)
        buffer->stream = safe_calloc(1 ,buffer->stream_size);  // Ahi deje el calloc/ reserva 1 buffer inicializado en 0
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
    if (buffer->offset + size > buffer->stream_size) {
        uint32_t new_size = buffer->offset + size;
        buffer->stream = safe_realloc(buffer->stream, new_size);
        buffer->stream_size = new_size;
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
	if (buffer->offset + size > buffer->stream_size)
	{
        LOG_WARN("Se quiere leer más de lo permitido ojito. Offset: %u, Size: %u, Buffer size: %u\n", buffer->offset, size, buffer->stream_size);
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
    char *string = safe_malloc(*length);
    buffer_read(buffer, string, *length);
    return string;
}

void* buffer_read_pointer(t_buffer *buffer) {
    uintptr_t ptr_as_integer;
    buffer_read(buffer, &ptr_as_integer, sizeof(uintptr_t)); // Leer el puntero como entero
    return (void*)ptr_as_integer; // Convertir de vuelta a un puntero
}
