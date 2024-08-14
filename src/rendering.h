#pragma once

#include <flecs.h>
#include <raylib.h>

typedef Vector2 WindowSize;

typedef struct
{
  char title[128];
} WindowTitle;

typedef struct
{
  short target;
} WindowFps;

extern ECS_COMPONENT_DECLARE (WindowSize);
extern ECS_COMPONENT_DECLARE (WindowTitle);
extern ECS_COMPONENT_DECLARE (WindowFps);

void RenderingModuleImport (ecs_world_t *world);

void SetupWindow (ecs_iter_t *it);
void CheckShouldClose (ecs_iter_t *it);
void BeginRendering (ecs_iter_t *it);
void StartRendering (ecs_iter_t *it);
void FinishRendering (ecs_iter_t *it);
