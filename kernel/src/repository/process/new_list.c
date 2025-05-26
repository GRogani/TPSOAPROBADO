#include "new_list.h"

static sem_t sem_new;

bool initialize_repository_new() {
    if (sem_init(&sem_new, 0, 1) != 0) {
        LOG_ERROR("sem_init for NEW list failed");
        exit(EXIT_FAILURE);
    }
}

bool destroy_repository_new() {
    sem_destroy(&sem_new);
}

bool find_and_lock_new_list(int process_id) {
    sem_wait(&sem_new);

    bool pid_matches(void* ptr) {
	    int el_pid = (int) ptr;
	    return el_pid == process_id;
	};

    void* el_found = list_find(get_new_list(), pid_matches);

    if(el_found == NULL) {
        return false;
    }

    return true;
}

void create_new(int pid) {
    list_add(get_new_list(), pid);
}

bool delete_new(int process_id) {

    bool pid_matches(void* ptr) {
        int el_pid = (int) ptr;
        return el_pid == process_id;
    }

    int removed_el = list_remove_by_condition(get_new_list(), pid_matches);

    if(removed_el != process_id) {
        return false;
    }

    return true;
}

void unlock_new_list() {
    sem_post(&sem_new);
}