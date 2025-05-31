#ifndef ENUM_PROCESS_STATE
#define ENUM_PROCESS_STATE

typedef enum {
    NEW,
    READY,
    EXEC,
    BLOCKED,
    SUSP_BLOCKED,
    SUSP_READY,
    EXIT_L,
    PROCESS_STATE_INVALID = -1
} PROCESS_STATE;

/**
 * @brief Convierte un estado de proceso a string para logging.
 * @param state Estado del proceso.
 * @return String que representa el estado.
 */
char* process_state_to_string(PROCESS_STATE state);

#endif
