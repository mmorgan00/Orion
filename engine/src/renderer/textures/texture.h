#pragma once

#include "core/logger.h"
#include "core/omemory.h"
#include "core/ostring.h"
#include "defines.h"

#include "platform/filesystem.h"

#include "renderer/renderer_types.inl"

b8 load_texture(const char *image_source, u32 *out_width, u32 *out_height,
                u32 **pixels);