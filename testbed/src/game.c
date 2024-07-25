#include "game.h"

#include <core/logger.h>
#include <core/omemory.h>
#include <renderer/renderer_frontend.h>
#include <core/input.h>

static u32 mesh_data_id;
b8 game_initialize(struct game* game_inst) {
  ODEBUG("game_initialize() called");
  return true;
}

  // Function pointer to game's update/loop
b8 game_update(struct game* game_inst, f32 delta_time) {
      static u64 alloc_count = 0;
    u64 prev_alloc_count = alloc_count;
    alloc_count = get_memory_alloc_count();
    if (input_is_key_up('M') && input_was_key_down('M')) {
        ODEBUG("Allocations: %llu (%llu this frame)", alloc_count, alloc_count - prev_alloc_count);
    }
    // TODO: Temp code, remove after testing
    if (input_is_key_up('R') && input_was_key_down('R')) {

      f32 f = 2.0f;
      vertex_3d plane[6];
      plane[0].position.x = -0.5 * f;
      plane[0].position.y = -0.5 * f;
      plane[0].tex_coord.u = 0.0;
      plane[0].tex_coord.v = 0.0;

      plane[1].position.x = 0.5 * f;
      plane[1].position.y = 0.5 * f;
      plane[1].tex_coord.u = 1.0;
      plane[1].tex_coord.v = 1.0;

      plane[2].position.x = -0.5 * f;
      plane[2].position.y = 0.5 * f;
      plane[2].tex_coord.u = 0.0;
      plane[2].tex_coord.v = 1.0;

      plane[3].position.x = 0.5 * f;
      plane[3].position.y = -0.5 * f;
      plane[3].tex_coord.u = 1.0;
      plane[3].tex_coord.v = 0.0;
      mesh_data_id = renderer_load_mesh(plane, 3);
      ODEBUG("%d", mesh_data_id);
      renderer_register_object(mesh_data_id, 2);
    }


  return true;
}

  // Function pointer to game's render pass
b8 game_render(struct game* game_inst, f32 delta_time) {
  return true;
}

  // Function pointer to handle resize, if applicable
void game_on_resize(struct game* game_inst, u32 width, u32 height) {

}
