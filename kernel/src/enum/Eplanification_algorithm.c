#include "Eplanification_algorithm.h"

static const char *enum_names[PLANIFICATION_ENUM_SIZE] = {"FIFO", "SJF", "SRT"};

PLANIFICATION_ALGORITHM planification_from_string(char *planification_algorithm) {
	
    if (planification_algorithm == NULL) return PLANIFICATION_INVALID;

	for (int i = 0; i < PLANIFICATION_ENUM_SIZE; i++) {
		if (strcasecmp(planification_algorithm, enum_names[i]) == 0){
			return i;
		}
	}

	return PLANIFICATION_INVALID;
}