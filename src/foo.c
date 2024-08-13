#define FLECS_FOO_IMPL

#include "foo.h"

#include "rendering.h"

void
Move (ecs_iter_t *it)
{
  Position *p = ecs_field (it, Position, 0);
  Velocity *v = ecs_field (it, Velocity, 1);
  const WindowSize *ws = ecs_singleton_get (it->world, WindowSize);

  for (int i = 0; i < it->count; i++)
    {
      p[i].x += v[i].x;
      p[i].y += v[i].y;

      if (p[i].x + 20.0f >= ws->dimensions.x || p[i].x - 20.0f <= 0)
        v[i].x *= -1;
      if (p[i].y + 20.0f >= ws->dimensions.y || p[i].y - 20.0f <= 0)
        v[i].y *= -1;
    }
}

void
FooModuleImport (ecs_world_t *world)
{
  ECS_MODULE (world, FooModule);

  ECS_META_COMPONENT (world, Position);
  ECS_META_COMPONENT (world, Velocity);

  ECS_SYSTEM (world, Move, EcsOnUpdate, Position, Velocity);

  ecs_entity_t e = ecs_entity (world, { .name = "Ball" });
  ecs_set (world, e, Position, { 100, 200 });
  ecs_set (world, e, Velocity, { 2, 4 });

  const Position *p = ecs_get (world, e, Position);
  printf ("[start] Pos: %.2f, %.2f\n", (double)p->x, (double)p->y);
}
