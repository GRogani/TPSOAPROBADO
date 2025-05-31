#include "exec_list.h"

static sem_t sem_exec;

void initialize_repository_exec() {
    if (sem_init(&sem_exec, 0, 1) != 0) {
        LOG_ERROR("sem_init for EXEC list failed");
        exit(EXIT_FAILURE);
    }
}

void destroy_repository_exec() {
    sem_destroy(&sem_exec);
}

void lock_exec_list() {
    sem_wait(&sem_exec);
}

void unlock_exec_list() {
    sem_post(&sem_exec);
}

bool find_pcb_in_exec(uint32_t pid) {
    bool pid_matches(void* ptr) {
        t_pcb* pcb = (t_pcb*) ptr;
        return pcb->pid == pid;
    };

    void* pcb_found = list_find(get_exec_list(), pid_matches);
    return pcb_found != NULL;
}

void add_pcb_to_exec(t_pcb* pcb) {
    if (pcb == NULL) return;
    
    pcb_change_state(pcb, EXEC);
    list_add(get_exec_list(), pcb);
}

t_pcb* remove_pcb_from_exec(uint32_t pid) {
    bool pid_matches(void* ptr) {
        t_pcb* pcb = (t_pcb*) ptr;
        return pcb->pid == pid;
    };

    t_pcb* removed_pcb = list_remove_by_condition(get_exec_list(), pid_matches);
    return removed_pcb;
}

t_pcb* get_executing_pcb() {
    if (list_size(get_exec_list()) == 0) {
        return NULL;
    }
    
    // Retorna el primer (y único) proceso en ejecución sin removerlo
    // TODO: esto está mal, como pueden haber varios procesos en ejecución por multiples instancias de CPUs, deberiamos ver alguna forma de obtener el proceso ejecutandose desde la cpu directamente en vez de esta lista.
    // esta lista la usaria nomas para calcular facilmente el grado de multiprogramacion y nada mas que eso.
    return list_get(get_exec_list(), 0);
}
