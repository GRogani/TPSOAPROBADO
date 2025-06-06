

// void* request_io(void* args) {
//   /**
//    * 1. obtener el pid, device_name y el tiempo del dto (en args)
//    * 2. verificar si existe el device_name en el diccionario de conexiones
//    * 3. si existe, obtener el link y lockear la cola de requests
//    *    3.1. mandar a correr en un hilo detachable, el proceso para pasar de BLOCKED A SUSP. BLOCKED
//    *    3.2. pasar el proceso de EXEC a BLOCKED
//    *    3.3. crear la request en la cola de requests para ese device
//    *    3.4. mandar a replanificar (correr corto plazo)
//    * 4. si no existe
//    *    4.1. eliminar de EXEC el proceso
//    *    4.2. lockear lista EXIT
//    *    4.3. llamar rutina para agregar el proceso a la lista de EXIT
//    */
// }