#include "pong.h"
#include "raylib.h"

ECS_COMPONENT_DECLARE (Position);
ECS_COMPONENT_DECLARE (Velocity);

ECS_TAG_DECLARE (Ball);

static ecs_world_t *world;

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

static void
Move (ecs_iter_t *it)
{
  Position *p = ecs_field (it, Position, 0);
  Velocity *v = ecs_field (it, Velocity, 1);

  for (int i = 0; i < it->count; i++)
    {
      p[i].x += v[i].x;
      p[i].y += v[i].y;

      if (p[i].x + 20.0f >= WINDOW_WIDTH || p[i].x - 20.0f <= 0)
        v[i].x *= -1;
      if (p[i].y + 20.0f >= WINDOW_HEIGHT || p[i].y - 20.0f <= 0)
        v[i].y *= -1;
    }
}

static void
Render (ecs_iter_t *it)
{
  const Position *p = ecs_field (it, Position, 0);

  BeginDrawing ();
  {
    ClearBackground (BLACK);
    for (int i = 0; i < it->count; i++)
      DrawCircle ((int)p[i].x, (int)p[i].y, 20.0, WHITE);
  }
  EndDrawing ();
}

void
setup_world (void)
{
  world = ecs_init ();

  ecs_set_threads (world, 8);
  ecs_log_set_level (-1);
  ecs_log_enable_colors (true);

  ecs_singleton_set (world, EcsRest, { 0 });
  ECS_IMPORT (world, FlecsStats);

  ECS_COMPONENT_DEFINE (world, Position);
  ecs_struct (world,
              { .entity = ecs_id (Position),
                .members = { { .name = "x", .type = ecs_id (ecs_f32_t) },
                             { .name = "y", .type = ecs_id (ecs_f32_t) } } });
  ECS_COMPONENT_DEFINE (world, Velocity);
  ecs_struct (world,
              { .entity = ecs_id (Velocity),
                .members = { { .name = "x", .type = ecs_id (ecs_f32_t) },
                             { .name = "y", .type = ecs_id (ecs_f32_t) } } });
  ECS_TAG_DEFINE (world, Ball);

  ECS_SYSTEM (world, CheckPause, EcsPreUpdate, 0);
  ECS_SYSTEM (world, Move, EcsOnUpdate, Position, Velocity, [none] Ball);
  ECS_SYSTEM (world, Render, EcsOnStore, [in] Position, [none] Ball);

  ecs_entity_t e = ecs_entity (world, { .name = "Ball#1" });
  ecs_set (world, e, Position, { 100, 200 });
  ecs_set (world, e, Velocity, { 2, 4 });
  ecs_add_id (world, e, Ball);
}

void
cleanup_world (void)
{
  assert (world);
  ecs_fini (world);
  world = NULL;
}

bool
run_game_loop (void)
{
  return ecs_progress (world, 0.0);
}
