#ifndef MEMORIA_H
#define MEMORIA_H

#include <commons/config.h>
#include <commons/log.h>
#include "../../utils/src/utils/config/t_configs.h"
#include "../../utils/src/utils/config/config.h"
#include "../../utils/src/utils/logger/logger.h"
#include "../../utils/src/utils/socket/server.h"
#include "../../utils/src/utils/socket/client.h"
#include "../../utils/src/utils/shutdown.h"

#define MEMORIA_DISPONIBLE 1024

t_log* logger;

// INICIO BORRAR CUANDO SE CONFIGUREN LOS OPCODES
typedef uint32_t OPCODE;
enum {
    HANDSHAKE = 0,
    OBTENER_ESPACIO_LIBRE  = 3
};
// FIN BORRAR
#endif

