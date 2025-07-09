[![Tests](https://github.com/sisoputnfrba/tp-2025-1c-Mi-Grupo-1234/actions/workflows/compilation.yml/badge.svg)](https://github.com/sisoputnfrba/tp-2025-1c-Mi-Grupo-1234/actions/workflows/test.yml)

# Documentacion de los modulos

## Kernel
- [Planificador de corto plazo](https://github.com/sisoputnfrba/tp-2025-1c-Mi-Grupo-1234/blob/79fc7db0f0418016d7463b508021c89747611c3f/kernel/info/shortScheduler.md)

## CPU
- 
## Memoria
- 
## IO
- 

---
# Protocolo de comunicacion entre modulos
### Estructura
La comunicacion entre modulos esta dada por una estructura de `paquetes` definida de la siguiente forma:
```c
struct {
    OPCODE opcode;       
    t_buffer* buffer;    
}t_package
```
siendo `OPCODE` un enum de los siguientess:
```c
enum
{
    FETCH,              // cpu -> memoria
    INSTRUCTION,        // memoria -> cpu
    LIST_INSTRUCTIONS,
    GET_FREE_SPACE,
    INIT_PROCESS,       // kernel -> memoria
    UNSUSPEND_PROCESS,
    KILL_PROCESS,       // kernel -> memoria
    SWAP,
    WRITE_MEMORY,
    READ_MEMORY,
    DUMP_MEMORY,        // kernel -> memoria

    CONFIRMATION,       // server -> client

    NEW_IO,             // io -> kernel
    IO_COMPLETION,      // io -> kernel
    SYSCALL,            // cpu -> kernel

    DISPATCH,           // cpu -> kernel
    INTERRUPT,          // kernel -> cpu
    CPU_CONTEXT,        // cpu -> kernel

    IO_OPERATION,       // cpu -> kernel
};
```
y siendo el `buffer` la siguiente estructura:
```c
struct {
    uint32_t stream_size;   // Tamaño total del buffer en bytes
    void* stream;           // Puntero al stream de datos serializados
    uint32_t offset;        // Offset actual para operaciones de lectura/escritura
} t_buffer;
```

### Operaciones
Para estandarizar las operaciones de comunicacion entre modulos con paquetes definimos el siguiente concepto: Data Transfer Packages (DTP).

Los cuales siguien el siguiente formato:

Nombre de archivo: `{OPCODE}_package.c/h`
```c 
t_package *create_{OPCODE}_package (uint32_t success);

int send_{OPCODE}_package (int socket, uint32_t success);

{OPCODE}_package_data read_{OPCODE}_package (t_package *package);

```
siendo `{OPCODE}_package_data` una estructura con los atributos necesarios para la lectura del paquete. Directamente relacionado con los tipos de datos que se envian en el buffer.


> [!NOTE]
> Si es un solo tipo de dato el enviado, no se utiliza una estructura, si no el tipo de dato en si mismo.
> 
> *Ejemplo*: Se envia solo un `uint32` en el buffer del paquete, entonces la funcion read devuelve `uint32`.


A su vez, tambien se utilizan operaciones mas genericas, tanto adentro de las anteriores como por fuera, como las siguientes:
```c
t_package* create_package(OPCODE opcode, t_buffer* buffer);

void destroy_package (t_package* package);

void* serialize_package(t_package* package, uint32_t* total_size);

int send_package(int socket, t_package* package);

t_package* recv_package(int socket);
```


> [!TIP]
> Por cada OPCODE existe un solo archivo DTP asociado.

---
## Entrega

Para desplegar el proyecto en una máquina Ubuntu Server, podemos utilizar el
script [so-deploy] de la cátedra:

```bash
git clone https://github.com/sisoputnfrba/so-deploy.git
cd so-deploy
./deploy.sh -r=release -p=utils -p=kernel -p=cpu -p=memoria -p=io "tp-{año}-{cuatri}-{grupo}"
```

El mismo se encargará de instalar las Commons, clonar el repositorio del grupo
y compilar el proyecto en la máquina remota.

> [!NOTE]
> Ante cualquier duda, pueden consultar la documentación en el repositorio de
> [so-deploy], o utilizar el comando `./deploy.sh --help`.

## Guías útiles

- [Cómo interpretar errores de compilación](https://docs.utnso.com.ar/primeros-pasos/primer-proyecto-c#errores-de-compilacion)
- [Cómo utilizar el debugger](https://docs.utnso.com.ar/guias/herramientas/debugger)
- [Cómo configuramos Visual Studio Code](https://docs.utnso.com.ar/guias/herramientas/code)
- **[Guía de despliegue de TP](https://docs.utnso.com.ar/guías/herramientas/deploy)**

[so-commons-library]: https://github.com/sisoputnfrba/so-commons-library
[so-deploy]: https://github.com/sisoputnfrba/so-deploy
