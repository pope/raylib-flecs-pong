#include "input.h"

#include <raylib.h>

static void
CheckPause (ecs_iter_t *it)
{
  if (IsKeyPressed (KEY_P))
    {
      if (ecs_has_id (it->world, EcsOnUpdate, EcsDisabled))
        ecs_remove_id (it->world, EcsOnUpdate, EcsDisabled);
      else
        ecs_add_id (it->world, EcsOnUpdate, EcsDisabled);
    }
}

void
InputModuleImport (ecs_world_t *world)
{
  ECS_MODULE (world, InputModule);

  ECS_SYSTEM (world, CheckPause, EcsPreUpdate, 0);
}
