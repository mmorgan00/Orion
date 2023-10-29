#pragma once

#include "defines.h"

typedef struct platform_state {
    void* internal_state; // "Generic" struct
} platform_state

b8 platform_startup (
    platform_state* plat_state,
    const char* application_name,
    i32 x,
    i32 y,
    i32 width,
    i32 height);

void platform_shutdown(platform_state* plat_state);

b8 platform_pump_message(platform_state* plat_state);

void* platform_allocate(u64 size, b8 aligned);
void platform_freeze(void* block, b8 aligned);
void* platform_zero_copy(void* block, u64 size);
