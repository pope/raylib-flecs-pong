#pragma once

#include <flecs.h>

typedef struct
{
  float x, y;
} Position, Velocity;

extern ECS_COMPONENT_DECLARE (Position);
extern ECS_COMPONENT_DECLARE (Velocity);

void FooModuleImport (ecs_world_t *world);

void Move (ecs_iter_t *it);
