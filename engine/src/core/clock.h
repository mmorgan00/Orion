#pragma once
#include "defines.h"

typedef struct clock {
  f64 start_time;
  f64 elapsed;
} clock;

// updates the clock. should be called just before checking elapsed
OAPI void clock_update(clock *clock);

// starts the clock. Resets elapsed time.
OAPI void clock_start(clock *clock);

// stops the clock. does not reset elapsed
OAPI void clock_stop(clock *clock);
