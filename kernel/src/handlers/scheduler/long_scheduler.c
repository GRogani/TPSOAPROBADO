#include <stddef.h>

void* run_long_scheduler(void)
{
  /**
   * Aca va la implementacion del algoritmo de LARGO plazo:
   * 1. con semaforos, nos aseguramos de lockear las listas de READY & SUSP. READY
   * 2. verificamos si existen elementos en la lista de SUSP. READY
   * 3. si existen, mandamos mensaje a memoria para de-suspenderlos
   * 4. si la memoria nos responde OK, los agregamos a la lista de READY
   * 5. si no, los dejamos en SUSP. READY y cortamos la ejecucion del algoritmo
   * 6. si no existen elementos en SUSP. READY, debemos deslockear la lista y lockear la lista de NEW.
   * 7. verificamos si existen elementos en la lista de NEW
   * 8. si existen, intentamos inicializarlos en memoria
   * 9. si la memoria nos responde OK, los agregamos a la lista de READY
   * 10. si la memoria nos responde ERROR, los dejamos en NEW y cortamos la ejecucion del algoritmo
   * 9. si no existen elementos en NEW, deslockeamos la lista y cortamos la ejecucion del algoritmo
   * 
   * TENE EN CUENTA QUE SE DEBEN INTENTAR INICIALIZAR EN LOOP LA MAYOR CANTIDAD DE PROCESOS POSIBLES HASTA QUE LA MEMORIA NO PUEDA INICIALIZAR MAS O QUE NO TENGAMOS MAS PROCESOS PARA INICIALIZAR EN LAS LISTAS CORRESPONDIENTES.
   * 
   * TENE EN CUENTA QUE VAMOS A TENER QUE TENER UN ALGORITMO DEFINIDO QUE NOS INDIQUE QUE PROCESO DEBEMOS INTENTAR INICIALIZAR PRIMERO EN MEMORIA, EN LAS LISTAS DE NEW Y SUSP. READY.
   * 
   * ESTOS ALGORITMOS DEBERIAN ESTAR EN OTRO LADO DEFINIDIOS, ESTA RUTINA SE ABSTRAE DE ESTO.
   */
   
  // TODO: Implementar algoritmo de planificación de largo plazo
  // Por ahora, retorna NULL como placeholder
  return NULL;
}