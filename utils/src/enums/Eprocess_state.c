#include "Eprocess_state.h"

char* process_state_to_string(PROCESS_STATE state) {
    switch (state) {
        case NEW:
            return "NEW";
        case READY:
            return "READY";
        case EXEC:
            return "EXEC";
        case BLOCKED:
            return "BLOCKED";
        case SUSP_BLOCKED:
            return "SUSP_BLOCKED";
        case SUSP_READY:
            return "SUSP_READY";
        case EXIT_L:
            return "EXIT_L";
        default:
            return "UNKNOWN";
    }
}
