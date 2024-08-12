#include "foo.h"

ECS_COMPONENT_DECLARE (Position);
ECS_COMPONENT_DECLARE (Velocity);

void
Move (ecs_iter_t *it)
{
  Position *p = ecs_field (it, Position, 0);
  const Velocity *v = ecs_field (it, Velocity, 1);

  for (int i = 0; i < it->count; i++)
    {
      p[i].x += v[i].x;
      p[i].y += v[i].y;
    }
}

void
FooModuleImport (ecs_world_t *world)
{
  ECS_MODULE (world, FooModule);

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

  ECS_SYSTEM (world, Move, EcsOnUpdate, Position, [in] Velocity);

  ecs_entity_t e = ecs_entity (world, { .name = "Ball" });
  ecs_set (world, e, Position, { 10, 20 });
  ecs_set (world, e, Velocity, { 1, 2 });

  const Position *p = ecs_get (world, e, Position);
  printf ("[start] Pos: %.2f, %.2f\n", (double)p->x, (double)p->y);
}
