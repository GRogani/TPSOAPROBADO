#include "pcb.h"

extern t_kernel_config kernel_config; // en globals.h

t_pcb* pcb_create(int32_t pid, int32_t pc, int32_t size, char* pseudocode_file) {
    t_pcb* pcb = safe_malloc(sizeof(t_pcb));
    if (pcb == NULL) {
        return NULL;
    }

    // Inicializar campos básicos
    pcb->pid = pid;
    pcb->pc = pc;
    pcb->size = size;
    pcb->current_state = NEW;

    // Inicializar archivo de pseudocódigo
    if (pseudocode_file != NULL) {
        size_t filename_len = strlen(pseudocode_file) + 1;
        pcb->pseudocode_file = safe_malloc(filename_len);
        if (pcb->pseudocode_file == NULL) {
            free(pcb);
            return NULL;
        }
        strcpy(pcb->pseudocode_file, pseudocode_file);
    } else {
        pcb->pseudocode_file = NULL;
    }

    // Inicializar métricas de estado (ME) a cero
    memset(&pcb->ME, 0, sizeof(t_state_metrics));
    
    // Inicializar métricas de tiempo (MT) a cero
    memset(&pcb->MT, 0, sizeof(t_time_metrics));

    // Inicializar tiempo de inicio del estado actual
    //gettimeofday(&pcb->state_start_time, NULL);
    pcb->state_start_time_ms = get_current_time_ms();
    
    // Incrementar contador del estado inicial (NEW)
    pcb->ME.new_count = 1;
    pcb->MT.last_cpu_burst_ms = kernel_config.default_estimated_cpu_burst_ms;

    LOG_OBLIGATORIO("## (%d) Se crea el proceso - Estado: NEW", pid);

    return pcb;
}

void pcb_destroy(t_pcb* pcb) {
    if (pcb != NULL) {
        // Liberar memoria del nombre del archivo si fue asignada
        if (pcb->pseudocode_file != NULL) {
            free(pcb->pseudocode_file);
        }
        free(pcb);
    }
}

void pcb_change_state(t_pcb* pcb, PROCESS_STATE new_state) {
    if (pcb == NULL) {
        return;
    }

    // Calcular tiempo transcurrido en el estado actual
    // struct timeval current_time;
    // gettimeofday(&current_time, NULL);

    uint64_t time_in_current_state = total_time_ms(pcb->state_start_time_ms);
    // Actualizar métricas de tiempo según el estado actual
    switch (pcb->current_state) {
        case NEW:
            pcb->MT.new_time_ms += time_in_current_state;
            break;
        case READY:
            pcb->MT.ready_time_ms += time_in_current_state;
            break;
        case EXEC:
            pcb->MT.exec_time_ms += time_in_current_state;
            break;
        case BLOCKED:
            pcb->MT.blocked_time_ms += time_in_current_state;
            break;
        case SUSP_BLOCKED:
            pcb->MT.susp_blocked_time_ms += time_in_current_state;
            break;
        case SUSP_READY:
            pcb->MT.susp_ready_time_ms += time_in_current_state;
            break;
        case EXIT_L:
            pcb->MT.exit_time_ms += time_in_current_state;
            break;
        default:
            break;
    }

    const char* estados[] = {"NEW", "READY", "EXEC", "BLOCKED", "SUSP_BLOCKED", "SUSP_READY", "EXIT"};
    LOG_OBLIGATORIO("## (%d) Pasa del estado %s al estado %s", pcb->pid, estados[pcb->current_state], estados[new_state]);
    // Cambiar al nuevo estado
    pcb->current_state = new_state;
    
    // Actualizar contador del nuevo estado
    switch (new_state) {
        case NEW:
            pcb->ME.new_count++;
            break;
        case READY:
            pcb->ME.ready_count++;
            break;
        case EXEC:
            pcb->ME.exec_count++;
            break;
        case BLOCKED:
            pcb->ME.blocked_count++;
            break;
        case SUSP_BLOCKED:
            pcb->ME.susp_blocked_count++;
            break;
        case SUSP_READY:
            pcb->ME.susp_ready_count++;
            break;
        case EXIT_L:
            pcb->ME.exit_count++;
            break;
        default:
            break;
    }

    // Actualizar tiempo de inicio del nuevo estado
    pcb->state_start_time_ms = get_current_time_ms();
}

uint64_t get_current_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}

//????
// uint64_t time_diff_ms(struct timeval start, struct timeval end) {
//     uint64_t start_ms = (uint64_t)(start.tv_sec) * 1000 + (uint64_t)(start.tv_usec) / 1000;
//     uint64_t end_ms = (uint64_t)(end.tv_sec) * 1000 + (uint64_t)(end.tv_usec) / 1000;
//     return end_ms - start_ms;
// }

uint64_t total_time_ms(u_int64_t start_time_ms)
{
    return (get_current_time_ms() - start_time_ms);
}

char* pcb_get_pseudocode_file(t_pcb* pcb) {
    if (pcb == NULL) {
        return NULL;
    }
    return pcb->pseudocode_file;
}
