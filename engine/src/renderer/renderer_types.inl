#pragma once

#include "defines.h"
#include "math/math_types.h"

typedef enum renderer_backend_type {
RENDERER_BACKEND_TYPE_VULKAN,
RENDERER_BACKEND_TYPE_OPENGL,
// TODO: Not supported yet. Will add later
// RENDERER_BACKEND_TYPE_DIRECTX
} renderer_backend_type;


typedef struct renderer_backend {
  struct platform_state* plat_state;
  u64 frame_number;
  
  b8 (*initialize)(struct renderer_backend* backend, const char* application_name, struct platform_state* plat_state);

  void (*shutdown)(struct renderer_backend* backend);

  void (*resized)(struct renderer_backend* backend, u16 width, u16 height);

  b8(*begin_frame)(struct renderer_backend* backend, f32 delta_time);
  void (*update_global_state)(mat4 projection, mat4 view, vec3 view_position, vec4 ambient_colour, i32 mode);
  b8(*end_frame)(struct renderer_backend* backend, f32 delta_time);

} renderer_backend;


typedef struct render_packet {
  f32 delta_time;
} render_packet;


/**
 * @brief Universal Buffer Object for information will be shared across shaders regardless of implementation language or graphics API.
 * @param view - view transformation
 * @param projection - view transformation
 * @param reserved0 - reserved for
 */
typedef struct global_uniform_object {
  mat4 view;
  mat4 projection;
} global_uniform_object;