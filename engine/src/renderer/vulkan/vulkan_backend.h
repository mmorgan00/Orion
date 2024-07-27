#pragma once

#include "renderer/renderer_backend.h"
#include "renderer/renderer_types.inl"
#include "resources/resource_types.h"

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

void vulkan_renderer_draw_geometry(geometry_render_data data);

b8 vulkan_renderer_create_geometry(geometry* geometry, u32 vertex_count, const vertex_3d* vertices, u32 index_count, const u32* indices);
void vulkan_renderer_destroy_geometry(geometry* geometry);

void vulkan_renderer_backend_create_texture();

b8 vulkan_renderer_backend_end_frame(renderer_backend *backend, f32 delta_time);
