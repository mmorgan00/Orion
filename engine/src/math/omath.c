#include "omath.h"
#include "platform/platform.h"

#include <math.h>
#include <stdlib.h>

static b8 rand_seeded = FALSE;

/**
 * Note that these are here in order to prevent having to import the
 * entire <math.h> everywhere.
 */
f32 osin(f32 x) {
    return sinf(x);
}

f32 ocos(f32 x) {
    return cosf(x);
}

f32 otan(f32 x) {
    return tanf(x);
}

f32 oacos(f32 x) {
    return acosf(x);
}

f32 osqrt(f32 x) {
    return sqrtf(x);
}

f32 oabs(f32 x) {
    return fabsf(x);
}

i32 orandom() {
    if (!rand_seeded) {
        srand((u32)platform_get_absolute_time());
        rand_seeded = TRUE;
    }
    return rand();
}

i32 orandom_in_range(i32 min, i32 max) {
    if (!rand_seeded) {
        srand((u32)platform_get_absolute_time());
        rand_seeded = TRUE;
    }
    return (rand() % (max - min + 1)) + min;
}

f32 okrandom() {
    return (float)orandom() / (f32)RAND_MAX;
}

f32 okrandom_in_range(f32 min, f32 max) {
    return min + ((float)orandom() / ((f32)RAND_MAX / (max - min)));
}
