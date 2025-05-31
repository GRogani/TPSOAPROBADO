#include "long_scheduler.h"

extern t_kernel_config kernel_config;

bool run_long_scheduler(void)
{
  log_info(get_logger(), "long_scheduler: Iniciando planificador de largo plazo");

  // Establecer conexión con memoria
  int memory_socket = connect_to_memory(&kernel_config);
  if (memory_socket == -1)
  {
    log_error(get_logger(), "long_scheduler: Error conectando con memoria");
    return false;
  }

  bool processes_initialized = false;

  // Fase 1: Procesar procesos suspendidos listos (SUSP_READY)
  log_debug(get_logger(), "long_scheduler: Procesando lista SUSP_READY");

  lock_ready_list();
  lock_susp_ready_list();

  while (true)
  {
    // Obtener siguiente proceso de SUSP_READY usando algoritmo configurado
    t_pcb *pcb = get_next_process_to_initialize_from_susp_ready();
    if (pcb == NULL)
    {
      log_debug(get_logger(), "long_scheduler: No hay más procesos en SUSP_READY");
      break;
    }

    log_info(get_logger(), "long_scheduler: Intentando des-suspender proceso PID=%d", pcb->pid);

    // Intentar de-suspender proceso en memoria
    // TODO: IMPLEMENTAR DESUSPENSION
    break; // TODO: Eliminar esta línea cuando se implemente la des-suspensión

    bool memory_ok = true;
    if (memory_ok)
    {
      // Éxito: mover de SUSP_READY a READY
      log_info(get_logger(), "long_scheduler: Proceso PID=%d des-suspendido exitosamente", pcb->pid);

      pcb->current_state = READY;
      add_pcb_to_ready(pcb);
      processes_initialized = true;

      log_info(get_logger(), "long_scheduler: Proceso PID=%d movido a READY", pcb->pid);
    }
    else
    {
      // Error: devolver proceso a SUSP_READY y terminar
      log_warning(get_logger(), "long_scheduler: No se pudo des-suspender proceso PID=%d, "
                                "memoria sin espacio suficiente",
                  pcb->pid);
      add_pcb_to_susp_ready(pcb);
      break;
    }
  }

  unlock_susp_ready_list();
  // Fase 2: Procesar procesos nuevos (NEW)
  log_debug(get_logger(), "long_scheduler: Procesando lista NEW");

  lock_new_list();

  while (true)
  {
    // Obtener siguiente proceso de NEW usando algoritmo configurado
    t_pcb *pcb = get_next_process_to_initialize_from_new();
    if (pcb == NULL)
    {
      log_debug(get_logger(), "long_scheduler: No hay más procesos en NEW");
      break;
    }

    log_info(get_logger(), "long_scheduler: Intentando inicializar proceso PID=%d", pcb->pid);

    // Intentar crear proceso en memoria (para inicialización usamos tamaño mock)
    bool memory_ok = create_process_in_memory(memory_socket, pcb->pid, pcb->size, pcb->pseudocode_file);

    if (memory_ok)
    {
      // Éxito: mover de NEW a READY
      log_info(get_logger(), "long_scheduler: Proceso PID=%d inicializado exitosamente", pcb->pid);

      pcb->current_state = READY;
      add_pcb_to_ready(pcb);
      processes_initialized = true;

      log_info(get_logger(), "long_scheduler: Proceso PID=%d movido a READY", pcb->pid);
    }
    else
    {
      // Error: devolver proceso a NEW y terminar
      log_warning(get_logger(), "long_scheduler: No se pudo inicializar proceso PID=%d, memoria sin espacio suficiente", pcb->pid);
      add_pcb_to_new(pcb);
      break;
    }
  }

  unlock_ready_list();
  unlock_new_list();

  // Cerrar conexión con memoria
  disconnect_from_memory(memory_socket);

  if (processes_initialized)
  {
    log_info(get_logger(), "long_scheduler: Planificador de largo plazo completado - Se inicializaron procesos exitosamente");
    return true; // indicamos que se inicializaron procesos, asi corremos el algoritmo de corto plazo.
  }
  else
  {
    log_info(get_logger(), "long_scheduler: Planificador de largo plazo completado - No se inicializaron nuevos procesos");
  }

  return false;
}