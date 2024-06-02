#pragma once

#include "renderer_types.inl"
#include "resources/resource_types.h"

struct static_mesh_data;
struct platform_state;

b8 renderer_initialize(const char *application_name,
                       struct platform_state *plat_state);
void renderer_shutdown();

void renderer_on_resized(u16 width, u16 height);

b8 renderer_draw_frame(render_packet *packet);


void renderer_create_texture(const char* name, b8 auto_release, i32 width, i32 height, const u8* pixels, struct texture* out_texture);

void renderer_destroy_texture(struct texture* texture);