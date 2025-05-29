void run_short_scheduler(void)
{
  /**
   * Aca va la implementacion del algoritmo de corto plazo
   * 
   * FUNCIÓN despachar_proceso():
    // Nombre de la función sugerido por el diagrama: get_free_cpu()
    // Comentario inicial: Obtener una CPU libre por el flag [int current_processing]

    // 1. Bloquear el semáforo de la CPU para evitar concurrencia
    lock(cpu_sem)

    // 2. ¿Hay algún proceso ejecutándose actualmente?
    SI (cpu_esta_en_proceso) ENTONCES
        // 2a. Sí, la CPU está ocupada. ¿Se permite el desalojo (preemption)?
        SI (desalojo_habilitado) ENTONCES
            // El diagrama es ambiguo aquí, pero la lógica implica que si se permite
            // el desalojo, el flujo continúa para reemplazar el proceso actual.
            // Se asume que el flujo sigue hacia find_next_ready().
            CONTINUAR
        SINO
            // 2b. No se permite desalojar. No se puede hacer nada.
            unlock(cpu_sem)
            RETORNAR // Finaliza la función
        FIN SI
    FIN SI

    // 3. Bloquear la cola de procesos listos (READY)
    lock(READY)

    // 4. Buscar el siguiente proceso en la cola de listos
    proceso_seleccionado = find_next_ready()

    // 5. ¿Se encontró un proceso en la cola READY?
    SI (proceso_seleccionado EXISTE) ENTONCES
        // 5a. Sí, se encontró un proceso para ejecutar.
        // ¿Hay que desalojar a un proceso que ya estaba en ejecución?
        SI (desalojo_habilitado Y cpu_estaba_en_proceso) ENTONCES
            // Interrumpir el proceso actual para moverlo de EXEC a READY
            send(interrupt)
            recv(interrupt) // Esperar a que la interrupción sea manejada

            // Comentario del diagrama: "pasamos el que estaba ejecutando de EXEC a READY"
            lock(EXEC)
            // Aquí iría la lógica para mover el proceso interrumpido a la cola READY
        SINO
            // No hay que desalojar a nadie o no está permitido.
            lock(EXEC)
        FIN SI

        // 6. Asignar el nuevo proceso a la CPU
        // Comentario del diagrama: "pasamos el que debe ejecutar ahora de READY -> EXEC"
        // Mover el proceso_seleccionado de la cola READY a la cola EXEC

        // Enviar la orden de ejecución a la CPU con el PID y el Program Counter (PC) OPCODE: EXECUTE
        send(dispatch, proceso_seleccionado.pid, proceso_seleccionado.pc)

        // Actualizar el estado para indicar qué proceso se está ejecutando ahora
        current_processing = proceso_seleccionado.pid

        // 7. Liberar todos los semáforos bloqueados
        unlock(READY)
        unlock(EXEC)
        unlock(cpu_sem)

    SINO
        // 5b. No, no se encontró ningún proceso en la cola READY.
        lock(EXEC)
        SI (NO hay_alguno_en_EXEC) ENTONCES
            // Si no hay nada en READY ni en EXEC, la CPU queda ociosa.
            current_processing = -1
        FIN SI

        // Liberar todos los semáforos bloqueados
        unlock(READY)
        unlock(EXEC)
        unlock(cpu_sem)
    FIN SI

    FIN FUNCIÓN
   */
}