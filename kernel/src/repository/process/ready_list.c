#include "ready_list.h"

static sem_t sem_ready;

bool initialize_repository_ready() {
    if (sem_init(&sem_ready, 0, 1) != 0) {
        LOG_ERROR("sem_init for READY list failed");
        exit(EXIT_FAILURE);
    }
    return true;
}

bool destroy_repository_ready() {
    sem_destroy(&sem_ready);
    return true;
}

void lock_ready_list() {
    sem_wait(&sem_ready);
}

void unlock_ready_list() {
    sem_post(&sem_ready);
}

bool find_pcb_in_ready(uint32_t pid) {
    bool pid_matches(void* ptr) {
        t_pcb* pcb = (t_pcb*) ptr;
        return pcb->pid == pid;
    };

    void* pcb_found = list_find(get_ready_list(), pid_matches);
    return pcb_found != NULL;
}

void add_pcb_to_ready(t_pcb* pcb) {
    if (pcb == NULL) return;
    
    pcb_change_state(pcb, READY);
    list_add(get_ready_list(), pcb);
}

t_pcb* remove_pcb_from_ready(uint32_t pid) {
    bool pid_matches(void* ptr) {
        t_pcb* pcb = (t_pcb*) ptr;
        return pcb->pid == pid;
    };

    t_pcb* removed_pcb = list_remove_by_condition(get_ready_list(), pid_matches);
    return removed_pcb;
}

t_pcb* get_next_pcb_from_ready() {
    // Para algoritmos FIFO - obtiene el primer elemento
    if (list_size(get_ready_list()) == 0) {
        return NULL;
    }
    
    return list_remove(get_ready_list(), 0); // Remueve y retorna el primer elemento
}
