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

void* platform_allocate(u64 size, b8 aligned); // malloc
void platform_freeze(void* block, b8 aligned); // free
// other operations we'll want to have
void* platform_zero_copy(void* block, u64 size);
void* platform_copy_memory(void* dest, const void* source, u64 size);
void* platform_set_memory(void* dest, i32 value, u64 size);

void platform_console_write(const char* message, u8 color);
void platform_console_write_error(const char* message, u8 color);

f64 platform_get_absolute_time();

// Blocks main thread, only used for giving time back to the OS.
// Not exported
void platform_sleep(u64 ms);