#include "blocked_list.h"

static sem_t sem_blocked;

void initialize_repository_blocked() {
    if (sem_init(&sem_blocked, 0, 1) != 0) {
        LOG_ERROR("sem_init for BLOCKED list failed");
        exit(EXIT_FAILURE);
    }
}

void destroy_repository_blocked() {
    sem_destroy(&sem_blocked);
}

void lock_blocked_list() {
    sem_wait(&sem_blocked);
}

void unlock_blocked_list() {
    sem_post(&sem_blocked);
}

void* find_pcb_in_blocked(int32_t pid) {
    bool pid_matches(void* ptr) {
        t_pcb* pcb = (t_pcb*) ptr;
        return pcb->pid == pid;
    };

    void* pcb_found = list_find(get_blocked_list(), pid_matches);
    return pcb_found;
}

void add_pcb_to_blocked(t_pcb* pcb) {
    if (pcb == NULL) return;
    
    pcb_change_state(pcb, BLOCKED);
    list_add(get_blocked_list(), pcb);
}

t_pcb* remove_pcb_from_blocked(int32_t pid) {
    bool pid_matches(void* ptr) {
        t_pcb* pcb = (t_pcb*) ptr;
        return pcb->pid == pid;
    };

    t_pcb* removed_pcb = list_remove_by_condition(get_blocked_list(), pid_matches);
    return removed_pcb;
}
