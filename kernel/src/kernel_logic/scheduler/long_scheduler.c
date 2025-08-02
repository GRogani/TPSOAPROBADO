#include "long_scheduler.h"

bool run_long_scheduler(void)
{
  extern t_kernel_config kernel_config;

  LOG_INFO("Iniciando planificador de largo plazo");

  // Establecer conexión con memoria
  int memory_socket = connect_to_memory(&kernel_config);
  if (memory_socket == -1)
  {
    LOG_ERROR("Error conectando con memoria");
    return false;
  }

  bool processes_initialized = false;

  LOG_INFO("Procesando lista SUSP_READY");

  while (1)
  {
    // Obtener siguiente proceso de SUSP_READY usando algoritmo configurado
    lock_susp_ready_list();
    t_pcb *pcb = get_next_process_to_initialize_from_susp_ready();

    if(pcb == NULL) {
      unlock_susp_ready_list();
      break;
    }

    // SWAP IN -> Intentar de-suspender proceso en memoria
    LOG_INFO("Intentando des-suspender proceso con PID %d", pcb->pid);

    send_swap_in_package(memory_socket, pcb->pid);
    t_package* response = recv_package(memory_socket);
    if (!read_confirmation_package(response)){
      // Error: devolver proceso a SUSP_READY y terminar
      LOG_WARNING("No se pudo des-suspender proceso con PID %d", pcb->pid);
      LOG_WARNING("Fallo de SWAP o memoria sin espacio suficiente");
      destroy_package(response);
      unlock_susp_ready_list();
      break;
    }

    // Éxito: mover de SUSP_READY a READY
    LOG_OBLIGATORIO("## des-suspendido de SUSP_READY a READY para PID %d", pcb->pid);
    LOG_INFO("Proceso con PID %d des-suspendido correctamente", pcb->pid);
    t_pcb *pcb_pop = remove_pcb_from_susp_ready(pcb->pid);
    unlock_susp_ready_list();

    lock_ready_list();
    add_pcb_to_ready(pcb_pop);
    unlock_ready_list();
    
    processes_initialized = true;
    
    LOG_INFO("Proceso con PID %d movido a READY", pcb->pid);
    destroy_package(response);
    
  }

  LOG_INFO("No hay mas procesos en SUSP_READY");

  lock_ready_list();
  lock_new_list();

  while (1)
  {
    t_pcb *pcb = get_next_process_to_initialize_from_new();
    if (pcb == NULL)
    {
      LOG_INFO("No hay mas procesos en NEW");
      break;
    }

    LOG_INFO("Intentando inicializar proceso con PID %d", pcb->pid);

    
    bool memory_ok = create_process(memory_socket, pcb->pid, pcb->size, pcb->pseudocode_file);

    if (memory_ok)
    {
      LOG_INFO("Proceso con PID %d inicializado correctamente", pcb->pid);
      t_pcb* pcb_pop = remove_pcb_from_new(pcb->pid);

      
      add_pcb_to_ready(pcb_pop);
      

      processes_initialized = true;

      LOG_INFO("Proceso con PID %d movido a READY", pcb->pid);
      continue;
    }
    else
    {
      LOG_WARNING("No se pudo inicializar proceso con PID %d, memoria sin espacio suficiente", pcb->pid);
      break;
    }
  }

  unlock_new_list();
  unlock_ready_list();

  disconnect_from_memory(memory_socket);

  if (processes_initialized)
  {
    LOG_INFO("Planificador de largo plazo finalizado - Se inicializaron procesos correctamente");
    return true; 
  }
  else
  {
    LOG_INFO("Planificador de largo plazo finalizado - No se inicializaron nuevos procesos");
    return false;
  }

  return false;
}