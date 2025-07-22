#include "dump-memory.h"

void dump_memory_request_handler(int client_socket, t_package *package)
{
  int32_t pid = read_dump_memory_package(package);
  LOG_OBLIGATORIO("## PID: %u - Memory Dump solicitado", pid);

  process_info *proc = process_manager_find_process(pid);
  if (proc == NULL)
  {
    LOG_ERROR("DUMP_MEMORY: Proceso PID %u no encontrado", pid);
    send_confirmation_package(client_socket, false);
    return;
  }

  t_list *frames = proc->allocated_frames;
  if (frames == NULL || list_size(frames) == 0)
  {
    LOG_ERROR("DUMP_MEMORY: Proceso PID %u no tiene frames asignados", pid);
    send_confirmation_package(client_socket, false);
    return;
  }

  LOG_INFO("DUMP_MEMORY: Proceso PID %u tiene %d frames asignados", pid, list_size(frames));

  FILE *dump_file = create_dump_file(proc->pid);
  if (dump_file == NULL)
  {
    LOG_ERROR("DUMP_MEMORY: Error al crear el archivo de dump para el PID %d", proc->pid);
    send_confirmation_package(client_socket, false);
    return;
  }

  LOG_INFO("DUMP_MEMORY: Creando archivo de dump para el PID %d", proc->pid);

  void *buffer = malloc(memoria_config.TAM_PAGINA);
  if (buffer == NULL)
  {
    LOG_ERROR("DUMP_MEMORY: Error al alocar buffer para lectura de páginas");
    fclose(dump_file);
    send_confirmation_package(client_socket, false);
    return;
  }

  bool dump_success = true;
  int32_t total_bytes_dumped = 0;
  int32_t allocated_frames_length = list_size(frames);

  for (int i = 0; i < allocated_frames_length; i++)
  {
    int32_t *frame_number = list_get(frames, i);
    if (frame_number == NULL)
    {
      LOG_ERROR("DUMP_MEMORY: Frame número %d es NULL", i);
      continue;
    }

    int32_t physical_address = (*frame_number) * memoria_config.TAM_PAGINA;
    read_from_user_space(physical_address, buffer, memoria_config.TAM_PAGINA);

    if (fwrite(buffer, 1, memoria_config.TAM_PAGINA, dump_file) != memoria_config.TAM_PAGINA)
    {
      LOG_ERROR("DUMP_MEMORY: Error al escribir contenido del frame %u en el archivo", *frame_number);
      dump_success = false;
      break;
    }

    total_bytes_dumped += memoria_config.TAM_PAGINA;
    LOG_DEBUG("DUMP_MEMORY: Frame %u (dirección física %u) volcado correctamente", *frame_number, physical_address);
  }

  free(buffer);
  fclose(dump_file);

  if (dump_success)
  {
    LOG_OBLIGATORIO("## PID: %u - Memory Dump completado. %u",
                    pid, total_bytes_dumped);
    send_confirmation_package(client_socket, true);
  }
  else
  {
    LOG_ERROR("DUMP_MEMORY: Error durante el dump de memoria para el PID %u", pid);
    send_confirmation_package(client_socket, false);
  }
}

FILE *create_dump_file(int32_t pid)
{
  time_t now = time(NULL);
  struct tm *local_time = localtime(&now);
  char filename[512];

  if (memoria_config.DUMP_PATH != NULL)
  {
    char mkdir_cmd[512];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", memoria_config.DUMP_PATH);
    system(mkdir_cmd);

    snprintf(filename, sizeof(filename), "%s/%u-%04d%02d%02d_%02d%02d%02d.dmp",
             memoria_config.DUMP_PATH, pid,
             local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday,
             local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
  }
  else
  {
    snprintf(filename, sizeof(filename), "%u-%04d%02d%02d_%02d%02d%02d.dmp",
             pid, local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday,
             local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
  }

  return fopen(filename, "wb");
}