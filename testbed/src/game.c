#include "game.h"

#include <core/logger.h>
#include <core/omemory.h>
#include <renderer/renderer_frontend.h>
#include <core/input.h>


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
      renderer_register_object(1, 2);
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
