#pragma once

#include "core/application.h"
#include "core/logger.h"
#include "game_types.h"

// Externally defined function to create the game
extern b8 create_game(game* out_game);

/**
 * The main entry for the application
 */
int main(void) {

  game game_inst;
  if (!create_game(&game_inst)) {
    OFATAL("Could not create game");
    return -1;
  }

  // Ensure function pointers are set
  if (!game_inst.render || !game_inst.update || !game_inst.initialize || !game_inst.on_resize) {
    OFATAL("The game's driver functions must be assigned");
    return -2;
  }

  // Initializtion
  if (!application_create(&game_inst)) {
    OINFO("Application failed to create\n");
    return 1;
  }

  // Being game
  if (!application_run()) {
    OINFO("Application did not shut down gracefully\n");
    return 2;
  }
  
  application_config config;
  config.start_pos_x = 100;
  config.start_pos_y = 100;
  config.start_width = 1280;
  config.start_height = 720;
  config.name = "Orion Engine Testbed";
  

  return 0;
}
