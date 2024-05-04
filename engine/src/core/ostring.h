#pragma once

#include "defines.h"

// Returns the length of the given string.
OAPI u64 string_length(const char *str);

OAPI char *string_duplicate(const char *str);

OAPI b8 strings_equal(const char *str0, const char *str1);
