void exit_process(void* args) {
  /**
   * 1. obtenermos el pid
   * 2. lockeamos la lista de EXEC y la de EXIT
   * 3. eliminamos el proceso de la lista de EXEC
   * 4. invocamos rutina de EXIT (que deberia: pasar el proceso a la lista de EXIT, liberar el PCB y loggear stats)
   * 5. deslockeamos EXEC y EXIT
   * 6. corremos largo plazo (como se liberó espacio del sistema, intentamos inicializar procesos nuevos)
   * 7. corremos corto plazo
   */

}