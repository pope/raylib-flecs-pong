#pragma once

#include <flecs.h>

#undef ECS_META_IMPL
#ifndef FLECS_FOO_IMPL
#define ECS_META_IMPL EXTERN // Ensure meta symbols are only defined once
#endif

ECS_STRUCT (Position, {
  float x;
  float y;
});

ECS_STRUCT (Velocity, {
  float x;
  float y;
});

void FooModuleImport (ecs_world_t *world);

void Move (ecs_iter_t *it);
