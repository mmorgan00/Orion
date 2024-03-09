#include <core/logger.h>
#include <core/asserts.h>

// TODO: Test
#include <platform/platform.h>

int main(void) {
  OFATAL("A fatal  message", "");
  OERROR("An error message", "");
  OINFO("An info message", "");
  ODEBUG("A debug message", "");
  OTRACE("A trace message", "");

  platform_state state;
  if(platform_startup(&state, "Orion Engine Testbed", 100, 100, 1280, 720)) {
    while(platform_pump_messages(&state)) {}
  }
  platform_shutdown(&state);

  return 0;
}
