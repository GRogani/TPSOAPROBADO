void run_medium_scheduler(void)
{
  /**
   * Aca va la implementacion del algoritmo de MEDIANO plazo
   * 1. recibe un pid y un tiempo, hace sleep de ese tiempo
   * 2. terminado el sleep, blockeamos la lista de BLOCKED y verificamos si está el proceso en la lista de BLOCKED
   * 3. si existe, pasamos el proceso a SUSP. BLOCKED
   * 4. hacemos el swap con la memoria
   * 5. corremos el largo plazo (porque acabamos de liberar espacio en memoria del sistema)
   * 6. si no existe, loggeamos y no hacemos nada (probablemente ya se resolvió el proceso)
   */
}