#include "input.h"

ECS_COMPONENT_DECLARE (KeyBinding);

void
InputModuleImport (ecs_world_t *world)
{
  ECS_MODULE (world, InputModule);

  ECS_COMPONENT_DEFINE (world, KeyBinding);
}
