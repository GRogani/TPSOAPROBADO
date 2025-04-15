#include "io-opcode.enum.h"

static char *enum_names[OPCPDE] = {"FIFO"};

t_io_opcode io_opcode_from_string(char *opcode) {
	for (int i = 0; i < IO_OPCODE_ENUM_SIZE; i++) {
		if (strcasecmp(opcode, enum_names[i]) == 0){
			return i;
		}
	}

	return -1;
}