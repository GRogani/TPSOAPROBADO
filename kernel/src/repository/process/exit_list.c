#include "exit_list.h"

static sem_t sem_exit;

void initialize_repository_exit() {
    if (sem_init(&sem_exit, 0, 1) != 0) {
        LOG_ERROR("sem_init for EXIT list failed");
        exit(EXIT_FAILURE);
    }
}

void destroy_repository_exit() {
    sem_destroy(&sem_exit);
}

void lock_exit_list() {
    sem_wait(&sem_exit);
}

void unlock_exit_list() {
    sem_post(&sem_exit);
}

bool find_pcb_in_exit (int32_t pid) {
    bool pid_matches(void* ptr) {
        t_pcb* pcb = (t_pcb*) ptr;
        return pcb->pid == pid;
    };

    void* pcb_found = list_find(get_exit_list(), pid_matches);
    return pcb_found != NULL;
}

void add_pcb_to_exit (t_pcb* pcb) {
    if (pcb == NULL) return;
    
    pcb_change_state(pcb, EXIT_L);
    list_add(get_exit_list(), pcb);
}

void remove_pcb_from_exit (int32_t pid) 
{
    bool pid_matches(void* ptr) 
    {
        t_pcb* pcb = (t_pcb*) ptr;
        return pcb->pid == pid;
    };

    t_pcb* removed_pcb = list_remove_by_condition( get_exit_list(), pid_matches);
    pcb_destroy(removed_pcb);
    
}