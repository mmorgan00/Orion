#pragma once

#include "defines.h"

struct platform_state;
struct vulkan_context;

b8 platform_create_vulkan_surface(
				  struct platform_state* plat_state,
				  struct vulkan_context* context);

// In easier terms, pointer to a string array. string = 1, array of string = 2, pointer of array of string = 3 *'s
void platform_get_required_extension_names(const char*** names_darray);

