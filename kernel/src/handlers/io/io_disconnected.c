void io_disconnected(void* args) {
  /**
    * 1. obtener la conexión por el socketId del args
    * 2. vemos el current-processing.
    * 3. si existe,
    *    3.1. Lo buscamos en la lista BLOCKED
    *    3.2. si existe, lo pasamos a la lista EXIT
    *    3.3. si no existe, lo buscamos en la lista SUSP. BLOCKED
    *    3.4. si existe, lo pasamos a la lista EXIT
    *    3.5. si no existe, loggeamos un error. (no deberia pasar)
    * 4. si no existe, el IO no estaba procesando nada. No hacemos nada.
    * 5. eliminamos la conexion de la lista de conexiones.
    */
  */
}