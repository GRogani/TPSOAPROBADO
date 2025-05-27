void init_proc(void *args)
{
  /**
   * 1. obtenermos el pid, fileName
   * 2. creamos el proceso en la lista NEW
   * 3. corremos el largo plazo (lo agregamos de NEW a READY), lo corremos en un hilo joineable y esperamos que termine.
   * 4. si el algoritmo es con desalojo, entonces corremos el corto plazo
   * 5. si el algoritmo es sin desalojo, entonces mandamos una respuesta de dispatch a la CPU para que continue ejecutando el proceso.
   * (PARA TODAS LAS OTRAS SYSCALLS, COMO EL PROCESO DE BLOCKEA, ENTONCES SI O SI MANDAMOS A PLANIFICAR, ESTE ES EL UNICO CASO BORDE Y NOSE SI LA SYSCALL DUMP_MEMORY tambien)
   */
}