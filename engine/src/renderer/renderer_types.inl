#pragma once

#include "defines.h"
#include "resources/resource_types.h"
#include "math/math_types.h"

typedef enum renderer_backend_type {
RENDERER_BACKEND_TYPE_VULKAN,
RENDERER_BACKEND_TYPE_OPENGL,
RENDERER_BACKEND_TYPE_DIRECTX
} renderer_backend_type;

/**
 * @brief Global uniform object for holding all information that will need to be shared across all (or at least most) shaders
*/
typedef struct global_uniform_object {
  mat4 projection;  // 64 bytes
  mat4 view;        // 64 bytes
  mat4 m_reserved0; // 64 bytes, reserved for future use
  mat4 m_reserved1; // 64 bytes, reserved for future use
} global_uniform_object;


/**
 * @brief Describes the various properties an object needs to be rendered.
 * For now, this JUST has diffuse color and some reserved fields
 * Later on, expect to add specular, metallicness, roughness, etc for more advanced rendering approaches
*/
typedef struct object_uniform_object {
  vec4 diffuse_color; 
  vec4 v_reserved0;
  vec4 v_reserved1;
  vec4 v_reserved2; // 3 reserved to keep us at 64 bytes (32 bits per value * 4 values per vec4 etc).
} object_uniform_object;

// Start of a 'object' abstraction in conjunction with a mesh associated with it
typedef struct geometry_render_data {
    u32 object_id;
    mat4 model;
    texture* textures[16];
} geometry_render_data;

typedef struct renderer_backend {
  struct platform_state* plat_state;
  u64 frame_number;
  
  b8 (*initialize)(struct renderer_backend* backend, const char* application_name, struct platform_state* plat_state);

  void (*shutdown)(struct renderer_backend* backend);

  void (*resized)(struct renderer_backend* backend, u16 width, u16 height);

  b8 (*begin_frame)(struct renderer_backend* backend, f32 delta_time);
  void (*update_global_state)(mat4 projection, mat4 view, vec3 view_position, vec4 ambient_color, i32 mode);
  b8 (*end_frame)(struct renderer_backend* backend, f32 delta_time);

  // this will change A LOT. Id isn't supported right now, but we want to be able to push things to shaders, 
  // and likely address it by a certain object ID, so we have to start somehwere
  void (*update_object)(geometry_render_data render_data); 
  
  void (*create_texture)(const char* name, b8 auto_release, i32 width, i32 height, i32 channel_count, const u8* pixels, b8 has_transparency, struct texture* out_texture);
  void (*destroy_texture)(struct texture* texture);

} renderer_backend;


typedef struct render_packet {
  f32 delta_time;
} render_packet;

