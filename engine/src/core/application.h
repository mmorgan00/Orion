#pragma once

#include "defines.h"

struct game;

typedef struct application_config {

  // starting window x position
  i16 start_pos_x;

  // starting window y position
  i16 start_pos_y;

  // starting window width
  i16 start_width;

    // starting window height
  i16 start_height;

  // application/window name
  char* name;
  
} application_config;

OAPI b8 application_create(struct game* game_inst);

OAPI b8 application_run();
