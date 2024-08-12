#pragma once

#include <flecs.h>
#include <raylib.h>

typedef struct
{
  KeyboardKey key;
} KeyBinding;

extern ECS_COMPONENT_DECLARE (KeyBinding);

void InputModuleImport (ecs_world_t *world);
