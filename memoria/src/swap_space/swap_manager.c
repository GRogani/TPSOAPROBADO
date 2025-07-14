#include "swap_manager.h"

// aca necesitamos armar el manager para swapear procesos
// este manager debe:
/**
 * 1. swapear un proceso especifico, esto significa, para ese proceso:
 *  - dentro de la lista de frames allocados, obtener todos y por cada uno de ellos calcular la direccion fisica a la que corresponde esa pagina en espacio de usuario (user_space)
 *  - por cada una de esas paginas, liberarla en espacio de usuario (user_space)
 *  - una vez liberada de espacio de usuario, deberá agregarla a swap (que será un archivo, pero que se deberia tratar similar a user_space, es decir, espacio contiguo en memoria)
 *  - es decir, es como mapear de un user_space hacia otro, pero con otros frames numbers.
 *  - cuando tengamos que agregar a swap el proceso, obtenemos el bitmap de frames del bitmap libres
 *  - por cada frame libre del swap, asignar al proceso. basicamente deberiamos limpiar la lista de frames asignados en un principio y sobreescribirla con los nuevos frames asignados dentro de swap
 *  - cuando se hace el unswap, se hace el proceso inverso, es decir, sacar los frames de swap y meterlos en user_space.
 * 
 * hay que imaginarse el user_space y swap, los dos similares en funcionamiento, como espacios de memoria contiguos.
 * el swap file no tiene un tamaño maximo
 * tanto el swap como el user_space tienen un bitmap de frames libres, que se van asignando y liberando.
 * tanto el swap como el user_space se manejan con frames, que son bloques de memoria de un tamaño fijo (por ejemplo, 32 bytes).
 *
 */