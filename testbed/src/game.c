#include "game.h"

#include <core/logger.h>


b8 game_initialize(struct game* game_inst) {
  ODEBUG("game_initialize() called");
  return TRUE;
}

  // Function pointer to game's update/loop
b8 game_update(struct game* game_inst, f32 delta_time) {
  return TRUE;
}

  // Function pointer to game's render pass
b8 game_render(struct game* game_inst, f32 delta_time) {
  return TRUE;
}

  // Function pointer to handle resize, if applicable
void game_on_resize(struct game* game_inst, u32 width, u32 height) {

}
