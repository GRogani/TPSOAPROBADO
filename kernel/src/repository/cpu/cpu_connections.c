#include "cpu_connections.h"

static sem_t sem_cpu_connections;

unsigned int id_generator = 0;

_Atomic unsigned int cpu_id_generator = 1;

void initialize_repository_cpu_connections()
{
  if (sem_init(&sem_cpu_connections, 0, 1) != 0)
  {
    LOG_ERROR("sem_init for CPU_CONNECTIONS dict failed");
    exit(EXIT_FAILURE);
  }
}

void destroy_repository_cpu_connections()
{
  sem_destroy(&sem_cpu_connections);
}

void lock_cpu_connections()
{
  sem_wait(&sem_cpu_connections);
}

void* get_cpu_connection_by_id(char *id) {
  void* cpu_connection = dictionary_get(get_cpu_connections_dict(), id);
  return cpu_connection;
}

void* get_cpu_connection_by_pid(int32_t pid) 
{
  t_list* cpus = get_all_cpu_connections();

  bool get_cpu_by_pid(void* ptr) {
    t_cpu_connection* cpu_connection = (t_cpu_connection*)ptr;
    return cpu_connection->current_process_executing == pid;
  };

  t_cpu_connection* cpu_connection = list_find(cpus, get_cpu_by_pid);
  list_destroy(cpus);

  return cpu_connection;
}

char * create_cpu_connection(int socket_interrupt, int socket_dispatch)
{
  t_cpu_connection* cpu_connection = safe_malloc(sizeof(t_cpu_connection));
  cpu_connection->dispatch_socket_id = socket_dispatch;
  cpu_connection->interrupt_socket_id = socket_interrupt;
  cpu_connection->current_process_executing = -1;
  
  char* id = string_itoa(cpu_id_generator);
  cpu_connection->id = cpu_id_generator;
  
  initialize_repository_cpu(&cpu_connection->cpu_exec_sem);
  
  dictionary_put(get_cpu_connections_dict(), id, cpu_connection);
  
  cpu_id_generator++;

  return id;
}

void free_cpu_connection(void *ptr)
{
  t_cpu_connection *cpu_connection = (t_cpu_connection *)ptr;

  close(cpu_connection->dispatch_socket_id);
  close(cpu_connection->interrupt_socket_id);

  destroy_repository_cpu(&cpu_connection->cpu_exec_sem);

  free(cpu_connection);
}

void remove_cpu_connection(char *id)
{
  dictionary_remove_and_destroy(get_cpu_connections_dict(), id, free_cpu_connection);
}

void unlock_cpu_connections()
{
  sem_post(&sem_cpu_connections);
}

t_list *get_all_cpu_connections() {
    return dictionary_elements(get_cpu_connections_dict());
}

void* get_first_available_cpu() {
    t_list* connections_list = dictionary_elements(get_cpu_connections_dict());
    
    // Helper function to check if CPU is available
    bool is_cpu_available(void* ptr) {
        t_cpu_connection* connection = (t_cpu_connection*)ptr;
        return connection->current_process_executing == -1;
    };
    
    t_cpu_connection* available_cpu = list_find(connections_list, is_cpu_available);
    list_destroy(connections_list);
    
    return available_cpu;
}