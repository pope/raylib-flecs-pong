#include <flecs.h>
#include <raylib.h>
#include <stdlib.h>

#include "pong.h"

int
main (void)
{
  SetConfigFlags (FLAG_MSAA_4X_HINT);
  InitWindow (WINDOW_WIDTH, WINDOW_HEIGHT, "Pope Pong");

  ecs_world_t *world = ecs_init ();
  ecs_set_threads (world, 2);
  ecs_log_set_level (-1);
  ecs_log_enable_colors (true);
  ecs_set_target_fps (world, 60);

  ecs_singleton_set (world, EcsRest, { 0 });
  ECS_IMPORT (world, FlecsStats);

  setup_pong (world);

  while (!WindowShouldClose () && ecs_progress (world, 0.0))
    {
    }

  ecs_fini (world);
  CloseWindow ();

  return EXIT_SUCCESS;
}
