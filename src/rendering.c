#include "rendering.h"

#include "foo.h"
#include <assert.h>
#include <raylib.h>

ECS_COMPONENT_DECLARE (WindowSize);
ECS_COMPONENT_DECLARE (WindowTitle);
ECS_COMPONENT_DECLARE (WindowFps);

void
SetupWindow (ecs_iter_t *it)
{
  const WindowTitle *wt = ecs_singleton_get (it->world, WindowTitle);
  const WindowSize *ws = ecs_singleton_get (it->world, WindowSize);
  const WindowFps *wfps = ecs_singleton_get (it->world, WindowFps);

  SetConfigFlags (FLAG_MSAA_4X_HINT);
  InitWindow ((int)ws->x, (int)ws->y, wt->title);
  SetTargetFPS (wfps->target);

  printf ("[rendering] width: %d, height: %d, title: '%s'\n", (int)ws->x,
          (int)ws->y, wt->title);
  printf ("[rendering] fps: %d\n", wfps->target);
}

void
CheckShouldClose (ecs_iter_t *it)
{
  if (WindowShouldClose ())
    ecs_quit (it->world);
}

void
BeginRendering (ecs_iter_t *it)
{
  (void)it;
  BeginDrawing ();
}

void
StartRendering (ecs_iter_t *it)
{
  ecs_entity_t e = ecs_lookup (it->world, "foo.module.Ball");
  assert (e);
  const Position *p = ecs_get (it->world, e, Position);
  assert (p);

  ClearBackground (BLACK);
  DrawCircle ((int)p->x, (int)p->y, 20.0, WHITE);
}

void
FinishRendering (ecs_iter_t *it)
{
  (void)it;
  EndDrawing ();
}

void
RenderingModuleImport (ecs_world_t *world)
{
  ECS_MODULE (world, RenderingModule);

  ECS_COMPONENT_DEFINE (world, WindowSize);
  ECS_COMPONENT_DEFINE (world, WindowTitle);
  ECS_COMPONENT_DEFINE (world, WindowFps);

  ecs_singleton_set (world, WindowSize, { .x = 1280, .y = 720 });
  ecs_singleton_set (world, WindowTitle, { .title = "Pong Thing" });
  ecs_singleton_set (world, WindowFps, { .target = 60 });

  ECS_SYSTEM (world, SetupWindow, EcsOnStart, );
  ECS_SYSTEM (world, CheckShouldClose, EcsOnLoad, );
  ECS_SYSTEM (world, BeginRendering, EcsPreUpdate, );
  // TODO(pope): Really this should be a system anyone can hook into.
  ECS_SYSTEM (world, StartRendering, EcsPreStore, );
  ECS_SYSTEM (world, FinishRendering, EcsOnStore, );
}
