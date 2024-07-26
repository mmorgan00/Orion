#pragma once

#include "defines.h"
#include "math/math_types.h"

typedef enum renderer_backend_type {
RENDERER_BACKEND_TYPE_VULKAN,
RENDERER_BACKEND_TYPE_OPENGL,
// TODO: Not supported yet. Will add later
// RENDERER_BACKEND_TYPE_DIRECTX
} renderer_backend_type;


typedef struct vertex_data {
  vertex_3d* vertices;
  u32 vertex_count;
  u32* indices;
  u32 index_count;
} vertex_data;

typedef struct renderer_backend {
  struct platform_state* plat_state;
  u64 frame_number;
  
  b8 (*initialize)(struct renderer_backend* backend, const char* application_name, struct platform_state* plat_state);

  void (*shutdown)(struct renderer_backend* backend);

  void (*resized)(struct renderer_backend* backend, u16 width, u16 height);

  b8(*begin_frame)(struct renderer_backend* backend, f32 delta_time);
  void (*update_global_state)(mat4 projection, mat4 view, vec3 view_position, vec4 ambient_colour, i32 mode);
  void (*draw_object)(struct renderer_backend* backend, vertex_data* vert_data);
  b8(*end_frame)(struct renderer_backend* backend, f32 delta_time);

} renderer_backend;


typedef struct render_packet {
  f32 delta_time;
} render_packet;

/**
 * @brief Render Object describing a single entity to draw.
 * Everything is stored as an ID to allow for instanced draws of the same mesh/texture data. 
 * @param id - ID of the object, used for renderer internal tracking
 * @param geometry_data - Vertex data ID. 
 * @param texture_id - Texture data ID.
 */
typedef struct render_object {
  u32 id;
  u32 geometry_data_id;
  u32 texture_data_id;
} render_object;

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
