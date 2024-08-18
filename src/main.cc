#include <flecs.h>
#include <raylib.h>
#include <stdlib.h>

#include "pong.h"

int main(void) {
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Pope Pong");

  flecs::world world;
  world.set_threads(2);
  world.set_target_fps(60);

  ecs_log_set_level(-1);
  ecs_log_enable_colors(true);

  world.set<flecs::Rest>({});
  world.import <flecs::stats>();

  setup_pong(world);

  while (!WindowShouldClose() && world.progress()) {
  }

  CloseWindow();

  return EXIT_SUCCESS;
}
