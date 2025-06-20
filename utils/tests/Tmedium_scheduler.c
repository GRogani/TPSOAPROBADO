#include "Tmedium_scheduler.h"

context(MediumScheduler) {
    describe("WhenPidIsStillBlocked") {
        before {
            // Setup: inicializar listas, mocks, etc.
        } end
        after {
            // TearDown: limpiar mocks, liberar memoria, borrar listas...
        } end
        
        it("should_move_pid_to_susp_blocked_and_call_long_scheduler") {
            // paso a paso, llamar run_medium_scheduler(pid, timer)
            // y luego verificar que el PCB ya no está en BLOCKED
            // y sí en SUSP_BLOCKED; que run_long_scheduler() fue invocado, etc.
        } end
    } end
    describe("WhenPidIsNotBlocked") {
        // Otro escenario: el PID ya no está en BLOCKED al expirar el timer
        // Esperamos que no haga nada concreto

    } end
}