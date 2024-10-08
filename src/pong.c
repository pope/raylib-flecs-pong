#include "pong.h"

#include <flecs.h>
#include <raylib.h>
#include <sys/param.h>

#define PADDLE_WIDTH 30
#define PADDLE_HEIGHT 160
#define PADDLE_GAP 10

#define BALL_RADIUS 20

#define CLAMP(v, min, max) (MIN ((max), MAX ((min), (v))))

typedef Vector2 Position;
typedef Vector2 Velocity;

ECS_STRUCT (Player, {
  int32_t up_key;
  int32_t down_key;
});

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
MoveBall (ecs_iter_t *it)
{
  Position *p = ecs_field (it, Position, 0);
  Velocity *v = ecs_field (it, Velocity, 1);

  for (int i = 0; i < it->count; i++)
    {
      p[i].x += v[i].x * it->delta_time;
      p[i].y += v[i].y * it->delta_time;

      if ((p[i].x + BALL_RADIUS >= WINDOW_WIDTH) | (p[i].x - BALL_RADIUS <= 0))
        v[i].x *= -1;
      if ((p[i].y + BALL_RADIUS >= WINDOW_HEIGHT)
          | (p[i].y - BALL_RADIUS <= 0))
        v[i].y *= -1;
    }
}

static void
MovePlayer (ecs_iter_t *it)
{
  Position *p = ecs_field (it, Position, 0);
  const Velocity *v = ecs_field (it, Velocity, 1);
  const Player *player = ecs_field (it, Player, 2);

  for (int i = 0; i < it->count; i++)
    {
      float dir = 0;
      if (IsKeyDown (player[i].up_key))
        dir--;
      if (IsKeyDown (player[i].down_key))
        dir++;

      p[i].y += v[i].y * dir * it->delta_time;
      p[i].y = CLAMP (p[i].y, PADDLE_GAP,
                      WINDOW_HEIGHT - PADDLE_HEIGHT - PADDLE_GAP);
    }
}

static void
MoveCpu (ecs_iter_t *it)
{
  Position *cpu_p = ecs_field (it, Position, 0);
  const Velocity *cpu_v = ecs_field (it, Velocity, 1);

  for (int i = 0; i < it->count; i++)
    {
      ecs_iter_t ball_it = ecs_query_iter (it->world, it->ctx);
      while (ecs_query_next (&ball_it))
        {
          const Position *ball_p = ecs_field (&ball_it, Position, 0);

          for (int j = 0; j < ball_it.count; j++)
            {
              float dir = (cpu_p[i].y + PADDLE_HEIGHT / 2.0f > ball_p[j].y)
                              ? -it->delta_time
                              : it->delta_time;
              cpu_p[i].y += cpu_v[i].y * dir;
            }
        }

      cpu_p[i].y = CLAMP (cpu_p[i].y, PADDLE_GAP,
                          WINDOW_HEIGHT - PADDLE_HEIGHT - PADDLE_GAP);
    }
}

static void
CheckCollisions (ecs_iter_t *it)
{
  const Position *ball_p = ecs_field (it, Position, 0);
  Velocity *ball_v = ecs_field (it, Velocity, 1);

  for (int i = 0; i < it->count; i++)
    {
      ecs_iter_t paddle_it = ecs_query_iter (it->world, it->ctx);
      while (ecs_query_next (&paddle_it))
        {
          const Position *paddle_p = ecs_field (&paddle_it, Position, 0);

          for (int j = 0; j < paddle_it.count; j++)
            {
              Rectangle r = { paddle_p[j].x, paddle_p[j].y, PADDLE_WIDTH,
                              PADDLE_HEIGHT };
              if (CheckCollisionCircleRec (ball_p[i], BALL_RADIUS, r))
                ball_v[i].x *= -1;
            }
        }
    }
}

static void
BeginRendering (ecs_iter_t *it)
{
  (void)it;
  BeginDrawing ();
  ClearBackground (BLACK);
}

static void
EndRendering (ecs_iter_t *it)
{
  (void)it;
  DrawFPS (WINDOW_WIDTH - 100, 20);
  EndDrawing ();
}

static void
RenderBackground (ecs_iter_t *it)
{
  (void)it;
  DrawLine (WINDOW_WIDTH / 2, 0, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 4, WHITE);
  DrawLine (WINDOW_WIDTH / 2, WINDOW_HEIGHT / 4 * 3, WINDOW_WIDTH / 2,
            WINDOW_HEIGHT, WHITE);
  DrawCircleLines (WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, WINDOW_HEIGHT / 4.0,
                   WHITE);
}

static void
RenderBall (ecs_iter_t *it)
{
  const Position *p = ecs_field (it, Position, 0);
  for (int i = 0; i < it->count; i++)
    DrawCircle ((int)p[i].x, (int)p[i].y, BALL_RADIUS, WHITE);
}
static void

RenderPaddle (ecs_iter_t *it)
{
  const Position *p = ecs_field (it, Position, 0);
  for (int i = 0; i < it->count; i++)
    DrawRectangle ((int)p[i].x, (int)p[i].y, PADDLE_WIDTH, PADDLE_HEIGHT,
                   WHITE);
}

void
setup_pong (ecs_world_t *world)
{
  // Definitions

  ECS_COMPONENT (world, Position);
  ecs_struct (world,
              { .entity = ecs_id (Position),
                .members = { { .name = "x", .type = ecs_id (ecs_f32_t) },
                             { .name = "y", .type = ecs_id (ecs_f32_t) } } });
  ECS_COMPONENT (world, Velocity);
  ecs_struct (world,
              { .entity = ecs_id (Velocity),
                .members = { { .name = "x", .type = ecs_id (ecs_f32_t) },
                             { .name = "y", .type = ecs_id (ecs_f32_t) } } });
  ECS_META_COMPONENT (world, Player);
  ECS_TAG (world, Ball);
  ECS_TAG (world, Paddle);
  ECS_TAG (world, Cpu);

  // Game objects

  {
    ecs_entity_t e = ecs_entity (world, { .name = "obj.Ball" });
    ecs_set (world, e, Position, { WINDOW_WIDTH / 2.0, WINDOW_HEIGHT / 2.0 });
    ecs_set (world, e, Velocity, { 7 * 60, 7 * 60 });
    ecs_add_id (world, e, Ball);
  }
  {
    ecs_entity_t e = ecs_entity (world, { .name = "obj.Player" });
    ecs_set (world, e, Position,
             { PADDLE_GAP, WINDOW_HEIGHT / 2.0 - PADDLE_HEIGHT / 2.0 });
    ecs_set (world, e, Velocity, { 0, 5 * 60 });
    ecs_set (world, e, Player, { KEY_W, KEY_S });
    ecs_add_id (world, e, Paddle);
  }
  {
    ecs_entity_t e = ecs_entity (world, { .name = "obj.Cpu" });
    ecs_set (world, e, Position,
             { WINDOW_WIDTH - PADDLE_WIDTH - PADDLE_GAP,
               WINDOW_HEIGHT / 2.0 - PADDLE_HEIGHT / 2.0 });
    ecs_set (world, e, Velocity, { 0, 5 * 60 });
    ecs_add_id (world, e, Paddle);
    ecs_add_id (world, e, Cpu);
  }

  // Systems
  {
    ecs_entity_t PreRendering = ecs_new_w_id (world, EcsPhase);
    ecs_set_name (world, PreRendering, "PreRendering");
    ecs_entity_t OnRendering = ecs_new_w_id (world, EcsPhase);
    ecs_set_name (world, OnRendering, "OnRendering");
    ecs_entity_t PostRendering = ecs_new_w_id (world, EcsPhase);
    ecs_set_name (world, PostRendering, "PostRendering");

    ecs_add_pair (world, PreRendering, EcsDependsOn, EcsOnStore);
    ecs_add_pair (world, OnRendering, EcsDependsOn, PreRendering);
    ecs_add_pair (world, PostRendering, EcsDependsOn, OnRendering);

    ECS_SYSTEM (world, CheckPause, EcsPreUpdate, 0);
    ECS_SYSTEM (world, MovePlayer, EcsOnUpdate,
                Position, [in] Velocity, [in] Player);
    ECS_SYSTEM (world, MoveBall, EcsOnUpdate, Position, Velocity, [none] Ball);
    ecs_system (
        world,
        { .entity = ecs_entity (
              world, { .name = "MoveCpu",
                       .add = ecs_ids (ecs_dependson (EcsOnUpdate)) }),
          .query.expr = "Position, [in] Velocity, [in] Cpu",
          .callback = MoveCpu,
          .ctx = ecs_query (world, { .expr = "Position, [none] Ball" }) });
    ecs_system (
        world,
        { .entity = ecs_entity (
              world, { .name = "CheckCollisions",
                       .add = ecs_ids (ecs_dependson (EcsOnValidate)) }),
          .query.expr = "[in] Position, Velocity, [none] Ball",
          .callback = CheckCollisions,
          .ctx = ecs_query (world, { .expr = "Position, [none] Paddle" }) });
    ECS_SYSTEM (world, BeginRendering, PreRendering, 0);
    ECS_SYSTEM (world, RenderBackground, OnRendering, 0);
    ECS_SYSTEM (world, RenderBall, OnRendering, [in] Position, [none] Ball);
    ECS_SYSTEM (world, RenderPaddle,
                OnRendering, [in] Position, [none] Paddle);
    ECS_SYSTEM (world, EndRendering, PostRendering, 0);
  }
}
