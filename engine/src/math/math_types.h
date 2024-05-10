#pragma once

#include "defines.h"


/**
 * Union to allow multiple access methods
 * Lets us declare as vec2_u v = [1.0f, 2.0f];
 * Access as v.x (1.0f), v.y (2.0f);
 * Also access as v.r, v.s, v.u, etc you get the point
 **/
typedef union vec2_u {
    // An array of x, y
    f32 elements[2];
    struct {
        union {
            // The first element.
            f32 x, r, s, u;
        };
        union {
            // The second element.
            f32 y, g, t, v;
        };
    };
} vec2;


typedef struct vec3_u {
    union {
        // An array of x, y, z
        f32 elements[3];
        struct {
            union {
                // The first element.
                f32 x, r, s, u;
            };
            union {
                // The second element.
                f32 y, g, t, v;
            };
            union {
                // The third element.
                f32 z, b, p, w;
            };
        };
    };
} vec3;

typedef union vec4_u {
#if defined(KUSE_SIMD)
    // Used for SIMD operations.
    alignas(16) __m128 data;
#endif
    // An array of x, y, z, w
    alignas(16) f32 elements[4];
    union {
        struct {
            union {
                // The first element.
                f32 x, r, s;
            };
            union {
                // The second element.
                f32 y, g, t;
            };
            union {
                // The third element.
                f32 z, b, p;
            };
            union {
                // The fourth element.
                f32 w, a, q;
            };
        };
    };
} vec4;

typdef vec4 quat;
