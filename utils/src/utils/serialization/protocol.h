#ifndef UTILS_PROTOCOL_H_
#define UTILS_PROTOCOL_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef enum{
    HANDSHAKE
}cod_op;

typedef struct
{
	int size;
	void* stream;
	uint32_t offset;
} t_buffer;

typedef struct
{
	cod_op codigo_operacion;
	t_buffer* buffer;
} t_paquete;

//FUNCIONES PARA SERIALIZAR
t_buffer* buffer_create(uint32_t size);
void buffer_destroy(t_buffer *buffer);
void buffer_add(t_buffer *buffer, void *data, uint32_t size);
void buffer_add_uint32(t_buffer *buffer, uint32_t data);
void buffer_add_string(t_buffer *buffer, uint32_t length, char *string);
void buffer_add_pointer(t_buffer *buffer, void *ptr);
//FUNCIONES PARA DESERIALIZAR
void buffer_read(t_buffer *buffer, void *data, uint32_t size);
uint32_t buffer_read_uint32(t_buffer *buffer);
char *buffer_read_string(t_buffer *buffer, uint32_t *length);
void* buffer_read_pointer(t_buffer *buffer);

#endif
