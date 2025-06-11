#include "Eplanification_algorithm.h"

PLANIFICATION_ALGORITHM planification_algorithm_from_string(const char *algorithm_str) 
{
	if (strcmp(algorithm_str, "FIFO") == 0) {
		return FIFO;
	} else if (strcmp(algorithm_str, "SJF") == 0) {
		return SJF;
	} else if (strcmp(algorithm_str, "PMCP") == 0) {
		return PMCP;
	} else {
		return FIFO;
	}
}