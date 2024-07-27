#pragma once

#include "math/math_types.h"


typedef struct texture {
    u32 id;
    u32 width;
    u32 height;
    u8 channel_count;
    b8 has_transparency;
    u32 generation;
    void* internal_data;
} texture;

#define GEOMETRY_NAME_MAX_LENGTH 256;

/**
 * @brief describes the 'location' of geometry with a friendly name for game use
 * @param id - game facing id
 * @param internal_id - renderer backend recognized id
 */
typedef struct geometry {
    u32 id;
    u32 internal_id;
    char name[GEOMETRY_NAME_MAX_LENGTH];
} geometry;