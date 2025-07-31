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
    LOG_INFO("Lockeada lista EXEC");
}

void unlock_exec_list() {
    sem_post(&sem_exec);
    LOG_INFO("DES-Lockeada lista EXEC");
}

void* find_pcb_in_exec(int32_t pid) {
    bool pid_matches(void* ptr) {
        t_pcb* pcb = (t_pcb*) ptr;
        return pcb->pid == pid;
    };

    void* pcb_found = list_find(get_exec_list(), pid_matches);
    return pcb_found;
}

void add_pcb_to_exec(t_pcb* pcb) {
    if (pcb == NULL) return;
    
    pcb_change_state(pcb, EXEC);
    list_add(get_exec_list(), pcb);
}

t_pcb* remove_pcb_from_exec(int32_t pid) {
    bool pid_matches(void* ptr) {
        t_pcb* pcb = (t_pcb*) ptr;
        return pcb->pid == pid;
    };

    t_pcb* removed_pcb = list_remove_by_condition(get_exec_list(), pid_matches);

    if(removed_pcb != NULL)  {
        removed_pcb->MT.last_cpu_burst_ms = total_time_ms(removed_pcb->state_start_time_ms);
    }

    return removed_pcb;
}