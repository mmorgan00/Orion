#pragma once

#include "core/application.h"

typedef struct game {

  // Application config
  application_config app_config;

  // Function pointer to game's init
  b8 (*initialize)(struct game *game_inst);

  // Function pointer to game's update/loop
  b8 (*update)(struct game *game_inst, f32 delta_time);

  // Function pointer to game's render pass
  b8 (*render)(struct game *game_inst, f32 delta_time);

  // Function pointer to handle resize, if applicable
  void (*on_resize)(struct game *game_inst, u32 width, u32 height);

  // Game-specific state. Created and managed by game code
  void *state;

  // Application state
  void *application_state;

} game;
