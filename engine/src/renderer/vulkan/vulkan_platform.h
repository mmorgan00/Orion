#pragma once

#include "defines.h"

// In easier terms, pointer to a string array. string = 1, array of string = 2, pointer of array of string = 3 *'s
void platform_get_required_extension_names(const char*** names_darray);
