#pragma once

#include "renderer/renderer_backend.h"
#include "renderer/renderer_types.inl"

b8 vulkan_renderer_backend_initialize(renderer_backend *backend,
                                      const char *application_name,
                                      struct platform_state *plat_state);
void vulkan_renderer_backend_shutdown(renderer_backend *backend);

void vulkan_renderer_backend_on_resized(renderer_backend *backend, u16 width,
                                        u16 height);

b8 vulkan_renderer_backend_begin_frame(renderer_backend *backend,
                                       f32 delta_time);

void vulkan_renderer_backend_update_global_state(mat4 projection, mat4 view,
                                                 vec3 view_position,
                                                 vec4 ambient_color, i32 mode);

void vulkan_renderer_draw_object(renderer_backend* backend, vertex_data* vert_data);

void vulkan_renderer_backend_create_texture();

b8 vulkan_renderer_backend_end_frame(renderer_backend *backend, f32 delta_time);
