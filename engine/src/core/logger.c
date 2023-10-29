#include "logger.h"

// TODO: temporary - will be removed when platform layer
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

b8 initalize_logging()
{
    //TODO: create log file
    return TRUE;
}
void shutdown_logging()
{
    //TODO: cleanup logging/write queued entries
}

OAPI void log_output(log_level level, const char* message, ...)
{
    const char * level_strings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: "};
    b8 is_error = level < 2;
    // Display console output immediately
    char out_message[32000];
    memset(out_message, 0, sizeof(out_message)); 
    // Log file output in batches
}