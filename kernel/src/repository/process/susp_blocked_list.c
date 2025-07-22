#include "susp_blocked_list.h"

static sem_t sem_susp_blocked;

void initialize_repository_susp_blocked() {
    if (sem_init(&sem_susp_blocked, 0, 1) != 0) {
        LOG_ERROR("sem_init for SUSP_BLOCKED list failed");
        exit(EXIT_FAILURE);
    }
}

void destroy_repository_susp_blocked() {
    sem_destroy(&sem_susp_blocked);
}

void lock_susp_blocked_list() {
    sem_wait(&sem_susp_blocked);
}

void unlock_susp_blocked_list() {
    sem_post(&sem_susp_blocked);
}

t_pcb *find_pcb_in_susp_blocked(int32_t pid)
{
    bool pid_matches(void* ptr) {
        t_pcb* pcb = (t_pcb*) ptr;
        return pcb->pid == pid;
    };

    void* pcb_found = list_find(get_susp_blocked_list(), pid_matches);
    return pcb_found;
}

void add_pcb_to_susp_blocked(t_pcb* pcb) {
    if (pcb == NULL) return;
    
    pcb_change_state(pcb, SUSP_BLOCKED);
    list_add(get_susp_blocked_list(), pcb);
}

t_pcb* remove_pcb_from_susp_blocked(int32_t pid) {
    bool pid_matches(void* ptr) {
        t_pcb* pcb = (t_pcb*) ptr;
        return pcb->pid == pid;
    };

    t_pcb* removed_pcb = list_remove_by_condition(get_susp_blocked_list(), pid_matches);
    return removed_pcb;
}
