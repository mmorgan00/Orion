#pragma once

#include "renderer_types.inl"
#include "defines.h"

struct static_mesh_data;
struct platform_state;

OAPI u32 renderer_register_object(u32 geometry_data_id, u32 texture_data_id);

b8 renderer_initialize(const char *application_name,
                       struct platform_state *plat_state);
void renderer_shutdown();

void renderer_on_resized(u16 width, u16 height);

b8 renderer_draw_frame(render_packet *packet);

