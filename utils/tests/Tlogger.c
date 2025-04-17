#include "Tlogger.h"

context (LoggerTests) {

    describe("Logger Initialization, Access and Destruction") {

        after {
            destroy_logger();
        } end

        it("should initialize logger correctly") {
            init_logger("test.log", "[TEST01]", LOG_LEVEL_INFO);
            t_log* logger = get_logger();
            should_ptr(logger) not be null;  
            should_string(logger->program_name) be equal to("[TEST01]");
        } end

        it("should not allow multiple logger initializations in the same process") {
            init_logger("test.log", "[TEST01]", LOG_LEVEL_INFO);
            t_log* logger1 = get_logger();
            init_logger("test.log", "[TEST02]", LOG_LEVEL_INFO);
            t_log* logger2 = get_logger();
            should_ptr(logger1) be equal to(logger2);
        } end

        it("should handle logger destruction and prevent further access") {
            init_logger("test.log", "[TEST01]", LOG_LEVEL_INFO);
            destroy_logger();

            should_ptr(get_logger()) be null;  // El logger debe ser NULL después de destruido
        } end

        it("should allow re-initialization after destruction") {
            init_logger("test.log", "[TEST01]", LOG_LEVEL_INFO);
            destroy_logger();  
            
            init_logger("test.log", "[TEST02]", LOG_LEVEL_DEBUG);  
            t_log* logger = get_logger();
            should_ptr(logger) not be null;
        } end

        it("should handle multiple destruction orders, even if its null") {
            destroy_logger();
            init_logger("test.log", "[TEST01]", LOG_LEVEL_INFO);
            destroy_logger();
            destroy_logger();
            destroy_logger();
            
            should_ptr(get_logger()) be null;
        } end

    } end

}
