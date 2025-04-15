# Documentación de Utilidades

## Índice

- [safe_alloc.h](#safe_alloch)
  - [safe_malloc](#safe_malloc)
  - [safe_realloc](#safe_realloc)
  - [safe_calloc](#safe_calloc)
- [utils_protocolo.h](#utils_protocoloh)
  - [t_buffer](#t_buffer)
  - [t_package](#t_package)
  - [buffer_create](#buffer_create)
  - [buffer_destroy](#buffer_destroy)
  - [buffer_add](#buffer_add)
  - [buffer_add_uint32](#buffer_add_uint32)
  - [buffer_add_string](#buffer_add_string)
  - [buffer_add_pointer](#buffer_add_pointer)
  - [buffer_read](#buffer_read)
  - [buffer_read_uint32](#buffer_read_uint32)
  - [buffer_read_string](#buffer_read_string)
  - [buffer_read_pointer](#buffer_read_pointer)
- [log_error_macro.h](#log_error_macroh)
  - [LOG_ERROR](#log_error)
  - [LOG_WARN](#log_warn)

---

## safe_alloc.h

### `safe_malloc`

```c
void* safe_malloc(size_t size);
```

**Descripción**  
Asigna memoria con `malloc()`. Aborta si falla.

**Parámetros**
- `size`: Tamaño en bytes.

**Retorna**  
Puntero asignado o aborta.

---

### `safe_realloc`

```c
void* safe_realloc(void* pointer, size_t size);
```

**Descripción**  
Reasigna memoria con `realloc()`. Aborta si falla.

**Parámetros**
- `pointer`: Memoria original.
- `size`: Nuevo tamaño.

**Retorna**  
Puntero reasignado o aborta.

---

### `safe_calloc`

```c
void* safe_calloc(size_t count, size_t size);
```

**Descripción**  
Asigna memoria inicializada en cero con `calloc()`. Aborta si falla.

**Parámetros**
- `count`: Elementos.
- `size`: Tamaño por elemento.

**Retorna**  
Puntero asignado o aborta.

---

## utils_protocolo.h

### `t_buffer`

```c
typedef struct t_buffer {
    uint32_t size;
    void* stream;
    uint32_t offset;
} t_buffer;
```

**Descripción**  
Buffer para serializar/deserializar datos.

- `size`: Tamaño total.  
- `stream`: Datos.  
- `offset`: Posición actual.

---

### `t_package`

```c
typedef struct t_package {
    OPCODE opcode;
    t_buffer* buffer;
} t_package;
```

**Descripción**  
Paquete con opcode y buffer.

- `opcode`: Tipo de operación.  
- `buffer`: Datos serializados.

---

### `buffer_create`

```c
t_buffer* buffer_create(uint32_t size);
```

**Descripción**  
Crea un buffer.

**Parámetros**
- `size`: Tamaño inicial.

**Retorna**  
Nuevo buffer.

---

### `buffer_destroy`

```c
void buffer_destroy(t_buffer *buffer);
```

**Descripción**  
Libera la memoria del buffer.

**Parámetros**
- `buffer`: Puntero al buffer.

---

### `buffer_add`

```c
void buffer_add(t_buffer *buffer, void *data, uint32_t size);
```

**Descripción**  
Agrega datos genéricos al buffer.

**Parámetros**
- `buffer`: Destino.
- `data`: Datos a copiar.
- `size`: Bytes a copiar.

---

### `buffer_add_uint32`

```c
void buffer_add_uint32(t_buffer *buffer, uint32_t data);
```

**Descripción**  
Agrega un entero de 32 bits.

**Parámetros**
- `buffer`: Destino.
- `data`: Valor entero.

---

### `buffer_add_string`

```c
void buffer_add_string(t_buffer *buffer, uint32_t length, char *string);
```

**Descripción**  
Agrega un string sin null-terminator.

**Parámetros**
- `buffer`: Destino.
- `length`: Longitud.
- `string`: Cadena.

---

### `buffer_add_pointer`

```c
void buffer_add_pointer(t_buffer *buffer, void *ptr);
```

**Descripción**  
Agrega un puntero crudo.

**Parámetros**
- `buffer`: Destino.
- `ptr`: Puntero.

**Nota**  
No válido en otra memoria o proceso.

---

### `buffer_read`

```c
void buffer_read(t_buffer *buffer, void *data, uint32_t size);
```

**Descripción**  
Lee `size` bytes del buffer.

**Parámetros**
- `buffer`: Fuente.
- `data`: Destino.
- `size`: Cantidad a leer.

---

### `buffer_read_uint32`

```c
uint32_t buffer_read_uint32(t_buffer *buffer);
```

**Descripción**  
Lee un `uint32_t`.

**Parámetros**
- `buffer`: Fuente.

**Retorna**  
Valor leído.

---

### `buffer_read_string`

```c
char* buffer_read_string(t_buffer *buffer, uint32_t *length);
```

**Descripción**  
Lee un string.

**Parámetros**
- `buffer`: Fuente.
- `length`: Longitud recibida.

**Retorna**  
Cadena (`malloc`), liberar con `free()`.

---

### `buffer_read_pointer`

```c
void* buffer_read_pointer(t_buffer *buffer);
```

**Descripción**  
Lee un puntero crudo.

**Parámetros**
- `buffer`: Fuente.

**Retorna**  
Puntero leído.

**Nota**  
Sólo válido si se deserializa en el mismo proceso.

---

## log_error_macro.h

### `LOG_ERROR`

```c
#define LOG_ERROR(fmt, ...)
```

**Descripción**  
Mensaje de error con contexto (archivo, línea, función).

**Parámetros**
- `fmt`: Formato.
- `...`: Argumentos.

---

### `LOG_WARN`

```c
#define LOG_WARN(fmt, ...)
```

**Descripción**  
Advertencia con contexto (archivo, línea, función).

**Parámetros**
- `fmt`: Formato.
- `...`: Argumentos.

