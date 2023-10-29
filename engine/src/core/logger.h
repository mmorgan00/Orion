#pragma once
#include "defines.h"

#define WARN_ENABLED 1;
#define INFO_ENABLED 1;
#define DEBUG_ENABLED 1;
#define TRACE_ENABLED 1;

#if ORELEASE == 1
#define DEBUG_ENABLED 0
#define TRACE_ENABLED 0
#endif

typedef enum log_level {
    LL_FATAL = 0,
    LL_ERROR = 1,
    LL_WARN = 2,
    LL_INFO = 3,
    LL_DEBUG = 4,
    LL_TRACE = 5
} log_level;


b8 initalize_logging();
void shutdown_logging();

OAPI void log_output(log_level level, const char* message, ...);

// Fatal level message
#define OFATAL(message, ...) log_level(LL_FATAL, message, #__VA_ARGS__);

#ifndef OERROR 
// error level message
#define OERROR(message, ...) log_level(LL_ERROR, message, #__VA_ARGS__);
#endif

#if WARN_ENABLED == 1
// error level message
#define OWARN(message, ...) log_level(LL_WARN, message, #__VA_ARGS__);
#else
#define OWARN(message, ...)
#endif

#if INFO_ENABLED == 1
// error level message
#define OINFO(message, ...) log_level(LL_INFO, message, #__VA_ARGS__);
#else
#define OINFO(message, ...)
#endif


#if DEBUG_ENABLED == 1
// error level message
#define ODEBUG(message, ...) log_level(LL_DEBUG, message, #__VA_ARGS__);
#else
#define ODEBUG(message, ...)
#endif

#if TRACE_ENABLED == 1
// error level message
#define OTRACE(message, ...) log_level(LL_TRACE, message, #__VA_ARGS__);
#else
#define OTRACE(message, ...)
#endif