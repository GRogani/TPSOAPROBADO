#include "init-process.h"

void init_process_request_handler(int socket, t_package *package)
{
  init_process_package_data *init_process_args = read_init_process_package(package);

  LOG_OBLIGATORIO("## PID: %u - Solicitud de creacion de Proceso Recibida.", init_process_args->pid);

  lock_process_creation();
  bool result = create_process(init_process_args->pid, init_process_args->size, init_process_args->pseudocode_path);
  unlock_process_creation();

  destroy_init_process_package(init_process_args);

  send_confirmation_package(socket, result);
}

bool create_process(int32_t pid, int32_t size, char *script_path)
{
  process_info *proc = safe_malloc(sizeof(process_info));

  proc->pid = pid;
  proc->process_size = size;
  proc->is_suspended = false;
  proc->instructions = process_manager_load_script_lines(script_path);

  if (proc->instructions == NULL)
  {
    LOG_ERROR("## PID: %u - Error al cargar pseudocodigo desde: %s", pid, script_path);
    free(proc);
    return false;
  }

  if (size != 0)
  {

    int32_t pages_needed = (int32_t)ceil((double)size / memoria_config.TAM_PAGINA);
    LOG_OBLIGATORIO("## PID: %u - Proceso requiere %d paginas (tamano: %d bytes, pagina: %d bytes).", pid, pages_needed, size, memoria_config.TAM_PAGINA);

    if (frame_get_free_count() < pages_needed)
    {
      LOG_WARNING("## PID: %u - No hay suficientes frames disponibles para crear el proceso. (Necesita %d, Libre %d)", pid, pages_needed, frame_get_free_count());
      if (proc->instructions)
        list_destroy_and_destroy_elements(proc->instructions, free);
      free(proc);
      return false;
    }

    proc->allocated_frames = allocate_frames(pages_needed);
    if (proc->allocated_frames == NULL || list_size(proc->allocated_frames) != pages_needed)
    {
      LOG_ERROR("## PID: %u - Error al asignar frames para el proceso.", pid);
      if (proc->instructions)
        list_destroy_and_destroy_elements(proc->instructions, free);
      free(proc);
      return false;
    }

    proc->page_table = init_page_table();
    if (proc->page_table == NULL)
    {
      LOG_ERROR("## PID: %u - Error al inicializar estructura de tabla de paginas.", pid);
      if (proc->instructions)
        list_destroy_and_destroy_elements(proc->instructions, free);
      release_frames(proc->allocated_frames);
      free(proc);
      return false;
    }

    bool success = assign_frames(proc->page_table, proc->allocated_frames, pages_needed);
    if (!success)
    {
      LOG_ERROR("## PID: %u - Error al asignar frames a las entradas de la tabla de paginas.", pid);
      if (proc->instructions)
        list_destroy_and_destroy_elements(proc->instructions, free);
      free_page_table(proc->page_table);
      release_frames(proc->allocated_frames);
      free(proc);
      return false;
    }
  }

  proc->metrics = malloc(sizeof(t_process_metrics));
  if (proc->metrics == NULL)
  {
    LOG_ERROR("## PID: %u - Error: No se pudo asignar memoria para metricas.", pid);
    if (proc->instructions)
      list_destroy_and_destroy_elements(proc->instructions, free);
    free_page_table(proc->page_table);
    release_frames(proc->allocated_frames);
    free(proc);
    return false;
  }
  memset(proc->metrics, 0, sizeof(t_process_metrics));

  lock_process_list();
  list_add(process_manager_get_process_list(), proc);
  unlock_process_list();

  LOG_OBLIGATORIO("## PID: %u - Proceso Creado - Tamaño: %u", pid, size);
  return true;
}

t_list *process_manager_load_script_lines(char *path)
{
  char full_path[512];
  snprintf(full_path, sizeof(full_path), "%s%s", memoria_config.PATH_INSTRUCCIONES, path);

  LOG_INFO("Intentando abrir archivo de pseudocodigo: %s", full_path);

  FILE *file = fopen(full_path, "r");
  if (!file)
  {
    LOG_ERROR("Script Load: Error al abrir archivo: %s", full_path);
    return NULL;
  }

  LOG_INFO("Archivo de pseudocodigo abierto correctamente: %s", full_path);

  t_list *list = list_create();
  char *line = NULL;
  size_t len = 0;
  ssize_t read_bytes;

  while ((read_bytes = getline(&line, &len, file)) != -1)
  {
    int32_t line_len = strlen(line);
    while (line_len > 0 && (line[line_len - 1] == '\n' || line[line_len - 1] == '\r' ||
                            line[line_len - 1] == ' ' || line[line_len - 1] == '\t'))
    {
      line[line_len - 1] = '\0';
      line_len--;
    }

    if (line_len > 0)
      list_add(list, strdup(line));
  }

  free(line);
  fclose(file);
  return list;
}