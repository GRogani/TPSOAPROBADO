#include "buffer.h"

t_buffer *buffer_create(int32_t size){

    t_buffer *buffer = safe_malloc(sizeof(t_buffer));

    buffer->offset = 0;
    buffer->stream_size = size;

    if (size !=0)
        buffer->stream = safe_calloc(1, buffer->stream_size);  // Ahi deje el calloc/ reserva 1 buffer inicializado en 0
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
        LOG_WARNING("Intentaste destruir un buffer NULL");
    }
}

void buffer_add(t_buffer *buffer, void *data, int32_t size){
    if (buffer->offset + size > buffer->stream_size) {
        int32_t new_size = buffer->offset + size;
        buffer->stream = safe_realloc(buffer->stream, new_size);
        buffer->stream_size = new_size;
    }

    memcpy(buffer->stream + buffer->offset, data, size);
    buffer->offset += size;
}


void buffer_add_int32(t_buffer *buffer, int32_t data){
    buffer_add(buffer, &data, sizeof(int32_t));
}

void buffer_add_string(t_buffer *buffer, int32_t length, char *string){
    buffer_add_int32(buffer, length);
    buffer_add(buffer, string, length);
}

void buffer_add_pointer(t_buffer *buffer, void *ptr) {
    uintptr_t ptr_as_integer = (uintptr_t)ptr; // Convertir el puntero a un entero sin signo
    buffer_add(buffer, &ptr_as_integer, sizeof(uintptr_t));
}


void buffer_read(t_buffer *buffer, void *data, int32_t size){
   	if (buffer == NULL)
	{
        LOG_WARNING("El buffer es NULL");
		return;
	}
    if (data == NULL)
    {
        LOG_WARNING("El puntero donde almacenar los datos es NULL");
        return;
    }

	// Verificar límites de lectura
	if (buffer->offset + size > buffer->stream_size)
	{
        LOG_WARNING("Se quiere leer más de lo permitido ojito. Offset: %u, Size: %u, Buffer size: %u", buffer->offset, size, buffer->stream_size);
		return;
	}

	memcpy(data, buffer->stream + buffer->offset, size);

	// Actualizar el desplazamiento del buffer
	buffer->offset += size;
}

int32_t buffer_read_int32(t_buffer *buffer){
    int32_t data;
    buffer_read(buffer, &data, sizeof(int32_t));
    return data;
}

char *buffer_read_string(t_buffer *buffer, int32_t *length){
    *length = buffer_read_int32(buffer);
    char *string = safe_malloc(*length);
    buffer_read(buffer, string, *length);
    return string;
}

void* buffer_read_pointer(t_buffer *buffer) {
    uintptr_t ptr_as_integer;
    buffer_read(buffer, &ptr_as_integer, sizeof(uintptr_t)); // Leer el puntero como entero
    return (void*)ptr_as_integer; // Convertir de vuelta a un puntero
}
