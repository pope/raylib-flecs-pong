#include <flecs.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

#include "foo.h"
#include "input.h"
#include "rendering.h"

int
main (void)
{
  ecs_world_t *world = ecs_init ();

  ecs_set_threads (world, 8);
  ecs_log_set_level (-1);
  ecs_log_enable_colors (true);

  ecs_singleton_set (world, EcsRest, { 0 });
  ECS_IMPORT (world, FlecsStats);

  ECS_IMPORT (world, InputModule);
  ECS_IMPORT (world, FooModule);
  ECS_IMPORT (world, RenderingModule);

  while (ecs_progress (world, 0.0))
    {
    }

  ecs_entity_t e = ecs_lookup (world, "foo.module.Ball");
  assert (e);
  const Position *p = ecs_get (world, e, Position);
  assert (p);

  printf ("[end] Pos: %.2f, %.2f\n", (double)p->x, (double)p->y);

  ecs_fini (world);
  CloseWindow ();

  return EXIT_SUCCESS;
}
