#include "susp_ready_list.h"

static sem_t sem_susp_ready;

void initialize_repository_susp_ready() {
    if (sem_init(&sem_susp_ready, 0, 1) != 0) {
        LOG_ERROR("sem_init for SUSP_READY list failed");
        exit(EXIT_FAILURE);
    }
}

void destroy_repository_susp_ready() {
    sem_destroy(&sem_susp_ready);
}

void lock_susp_ready_list() {
    sem_wait(&sem_susp_ready);
}

void unlock_susp_ready_list() {
    sem_post(&sem_susp_ready);
}

bool find_pcb_in_susp_ready(int32_t pid) {
    bool pid_matches(void* ptr) {
        t_pcb* pcb = (t_pcb*) ptr;
        return pcb->pid == pid;
    };

    void* pcb_found = list_find(get_susp_ready_list(), pid_matches);
    return pcb_found != NULL;
}

void add_pcb_to_susp_ready(t_pcb* pcb) {
    if (pcb == NULL) return;
    
    pcb_change_state(pcb, SUSP_READY);
    list_add(get_susp_ready_list(), pcb);
}

t_pcb* remove_pcb_from_susp_ready(int32_t pid) {
    bool pid_matches(void* ptr) {
        t_pcb* pcb = (t_pcb*) ptr;
        return pcb->pid == pid;
    };

    t_pcb* removed_pcb = list_remove_by_condition(get_susp_ready_list(), pid_matches);
    return removed_pcb;
}

t_pcb* get_next_pcb_from_susp_ready() {
    // Para algoritmos FIFO - obtiene el primer elemento
    if (list_size(get_susp_ready_list()) == 0) {
        return NULL;
    }
    
    return list_get(get_susp_ready_list(), 0); // Remueve y retorna el primer elemento
}
