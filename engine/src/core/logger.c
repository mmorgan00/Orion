#include "logger.h"
#include "asserts.h"
#include "ostring.h"
#include "platform/filesystem.h"
#include "platform/platform.h"

// TODO: temporary
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

typedef struct logger_system_state {
  file_handle log_file_handle;
} logger_system_state;

static logger_system_state *state_ptr;

void append_to_log_file(const char *message) {
  if (state_ptr && state_ptr->log_file_handle.is_valid) {
    u64 length = string_length(message);
    u64 written = 0;
    if (!filesystem_write(&state_ptr->log_file_handle, length, message,
                          &written)) {
      platform_console_write_error("ERROR writing to console.log.",
                                   LOG_LEVEL_ERROR);
    }
  }
}

b8 initialize_logging(u64 *memory_requirement, void *state) {
  *memory_requirement = sizeof(logger_system_state);
  if (state == 0) {
    return true;
  }

  state_ptr = state;

  // Create new (wipe if needed) log file, then open it
  if (!filesystem_open("console.log", FILE_MODE_WRITE, false,
                       &state_ptr->log_file_handle)) {
    platform_console_write_error(
        "ERROR: Unable to open console.log for writing", LOG_LEVEL_ERROR);
  }

  // TODO: Remove this
  OFATAL("A test message: %f", 3.14f);
  OERROR("A test message: %f", 3.14f);
  OWARN("A test message: %f", 3.14f);
  OINFO("A test message: %f", 3.14f);
  ODEBUG("A test message: %f", 3.14f);
  OTRACE("A test message: %f", 3.14f);
  return true;
}

void shutdown_logging(void *state) {
  // TODO: cleanup logging/write queued entries.
  state_ptr = 0;
}

void log_output(log_level level, const char *message, ...) {
  const char *level_strings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ",
                                  "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};
  b8 is_error = level < LOG_LEVEL_WARN;

  // Technically imposes a 32k character limit on a single log entry, but...
  // DON'T DO THAT!
  const i32 msg_length = 32000;
  char out_message[32000];
  memset(out_message, 0, sizeof(out_message));

  // Format original message.
  // NOTE: Oddly enough, MS's headers override the GCC/Clang va_list type with a
  // "typedef char* va_list" in some cases, and as a result throws a strange
  // error here. The workaround for now is to just use __builtin_va_list, which
  // is the type GCC/Clang's va_start expects.
  va_list arg_ptr;
  va_start(arg_ptr, message);
  vsnprintf(out_message, msg_length, message, arg_ptr);
  va_end(arg_ptr);

  char out_message2[32000];
  sprintf(out_message2, "%s%s\n", level_strings[level], out_message);

  // TODO: platform-specific output.
  if (is_error) {
    platform_console_write_error(out_message2, level);
  } else {
    platform_console_write(out_message2, level);
  }
}

void report_assertion_failed(const char *expression, const char *message,
                             const char *file, i32 line) {
  log_output(LOG_LEVEL_FATAL,
             "Assertion failure: %s, message: '%s', in file: %s, line: %d\n",
             expression, message, file, line);
}
