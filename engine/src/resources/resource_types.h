#pragma once

#include "math/math_types.h"

typedef struct texture {
    // future proofing - this should be helpful of 'which texture an object in game needs'
    u32 id;
    // standard details needed
    u32 width;
    u32 height;
    // channel/transparency seems like something that will be helpful later based on referencing unreal
    u8 channel_count;
    b8 has_transparency;
    // void pointer to be flexible of different renderer backends
    void* internal_data;
} texture;