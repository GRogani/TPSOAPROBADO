#include "planification-algorithm.enum.h"

static char *enum_names[PLANIFICATION_ENUM_SIZE] = {"FIFO", "SJF", "SRT"};

t_planification_algorithm planification_from_string(char *planification_algorithm) {
	for (int i = 0; i < PLANIFICATION_ENUM_SIZE; i++) {
		if (strcasecmp(planification_algorithm, enum_names[i]) == 0){
			return i;
		}
	}

	return -1;
}