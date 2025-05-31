#include "Eplanification_algorithm.h"

static char *short_planification_enum_names[3] = {"FIFO", "SJF", "SRT"};
static char *ready_planification_enum_names[2] = {"FIFO", "PMCP"};

PLANIFICATION_ALGORITHM short_planification_from_string(char *planification_algorithm) {
	
  if (planification_algorithm == NULL) return PLANIFICATION_INVALID;

	for (int i = 0; i < PLANIFICATION_ENUM_SIZE; i++) {
		if (strcasecmp(planification_algorithm, short_planification_enum_names[i]) == 0){
			return i;
		}
	}

	return PLANIFICATION_INVALID;
}

PLANIFICATION_ALGORITHM ready_planification_from_string(char *planification_algorithm) {
	
    if (planification_algorithm == NULL) return PLANIFICATION_INVALID;

	for (int i = 0; i < PLANIFICATION_ENUM_SIZE; i++) {
		if (strcasecmp(planification_algorithm, ready_planification_enum_names[i]) == 0){
			return i;
		}
	}

	return PLANIFICATION_INVALID;
}