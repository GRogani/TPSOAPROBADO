#include "new_list.h"
#include "repository/pcb/pcb.h"
#include <commons/collections/list.h>
#include <stdlib.h>

static sem_t sem_new;

void initialize_repository_new() {
    if (sem_init(&sem_new, 0, 1) != 0) {
        LOG_ERROR("sem_init for NEW list failed");
        exit(EXIT_FAILURE);
    }
}

void destroy_repository_new() {
    sem_destroy(&sem_new);
}

void lock_new_list() {
    sem_wait(&sem_new);
}

void unlock_new_list() {
    sem_post(&sem_new);
}

bool find_pcb_in_new(int32_t pid) {
    t_list* list = get_new_list();
    
    for (int i = 0; i < list_size(list); i++) {
        t_pcb* pcb = (t_pcb*) list_get(list, i);
        if (pcb && pcb->pid == pid) {
            return true;
        }
    }
    
    return false;
}

void add_pcb_to_new(t_pcb* pcb) {
    if (pcb == NULL) return;
    
    // El PCB ya debe estar en estado NEW al ser creado
    list_add(get_new_list(), pcb);
}

t_pcb* get_next_pcb_from_new() {
    // Para algoritmos FIFO - obtiene y remueve el primer elemento
    if (list_size(get_new_list()) == 0) {
        return NULL;
    }
    
    return (t_pcb*) list_get(get_new_list(), 0);
}

t_pcb* remove_pcb_from_new(int32_t pid) {
    bool pid_matches(void* ptr) {
        t_pcb* pcb = (t_pcb*) ptr;
        return pcb->pid == pid;
    };

    t_pcb* removed_pcb = list_remove_by_condition(get_new_list(), pid_matches);
    return removed_pcb;
}