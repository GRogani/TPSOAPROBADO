#include "new_list.h"
#include "../pcb.h"
#include <commons/collections/list.h>
#include <stdlib.h>
#include "macros/log_error.h"

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

bool find_pcb_in_new(uint32_t pid) {
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

t_pcb* remove_pcb_from_new(uint32_t pid) {
    t_list* list = get_new_list();
    
    for (int i = 0; i < list_size(list); i++) {
        t_pcb* pcb = (t_pcb*) list_get(list, i);
        if (pcb && pcb->pid == pid) {
            return (t_pcb*) list_remove(list, i);
        }
    }
    
    return NULL;
}

t_pcb* get_next_pcb_from_new() {
    // Para algoritmos FIFO - obtiene el primer elemento
    if (list_size(get_new_list()) == 0) {
        return NULL;
    }
    
    return (t_pcb*) list_get(get_new_list(), 0);
}