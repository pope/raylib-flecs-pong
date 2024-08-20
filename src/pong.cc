#include "pong.h"

#include <flecs.h>
#include <raylib.h>
#include <sys/param.h>

#define PADDLE_WIDTH 30
#define PADDLE_HEIGHT 160
#define PADDLE_GAP 10

#define BALL_RADIUS 20

#define CLAMP(v, min, max) (MIN((max), MAX((min), (v))))

struct Position {
  float x, y;
};

struct Velocity {
  float x, y;
};

struct Player {
  int32_t up_key, down_key;
};

struct Ball {};
struct Paddle {};
struct Cpu {};

static void CheckPause(flecs::iter &it) {
  if (IsKeyPressed(KEY_P)) {
    auto e = it.world().entity(flecs::OnUpdate);
    if (e.has(flecs::Disabled))
      e.remove(flecs::Disabled);
    else
      e.add(flecs::Disabled);
  }
}

static void MoveBall(flecs::iter &it, size_t i, Position &p, Velocity &v) {
  (void)i;
  p.x += v.x * it.delta_time();
  p.y += v.y * it.delta_time();

  if ((p.x + BALL_RADIUS >= WINDOW_WIDTH) | (p.x - BALL_RADIUS <= 0)) {
    v.x *= -1;
  }
  if ((p.y + BALL_RADIUS >= WINDOW_HEIGHT) | (p.y - BALL_RADIUS <= 0)) {
    v.y *= -1;
  }
}

static void MovePlayer(flecs::iter &it, size_t i, Position &p,
                       const Velocity &v, const Player &player) {
  (void)i;
  char dir = 0;
  if (IsKeyDown(player.up_key)) dir--;
  if (IsKeyDown(player.down_key)) dir++;

  p.y += v.y * dir * it.delta_time();
  p.y = CLAMP(p.y, PADDLE_GAP, WINDOW_HEIGHT - PADDLE_HEIGHT - PADDLE_GAP);
}

static void MoveCpu(flecs::iter &it, size_t i, Position &cpu_p,
                    const Velocity &cpu_v) {
  (void)i;
  // TODO(pope): Figure out how to use a query as a context.
  // My current issue is that the created query will go out of scope and then
  // get destructed.
  auto ball_q = it.world().query_builder<const Position>().with<Ball>().build();
  ball_q.each([&](const Position &ball_p) {
    char dir = (cpu_p.y + PADDLE_HEIGHT / 2.0f > ball_p.y) ? -1 : 1;
    cpu_p.y += cpu_v.y * dir * it.delta_time();
  });
  cpu_p.y =
      CLAMP(cpu_p.y, PADDLE_GAP, WINDOW_HEIGHT - PADDLE_HEIGHT - PADDLE_GAP);
}

static void CheckCollisions(flecs::iter &it, size_t i, const Position &ball_p,
                            Velocity &ball_v) {
  (void)i;
  // TODO(pope): Figure out how to use a query as a context.
  auto collision_q =
      it.world().query_builder<const Position>().with<Paddle>().build();
  collision_q.each([&](const Position &paddle_p) {
    Rectangle r = {paddle_p.x, paddle_p.y, PADDLE_WIDTH, PADDLE_HEIGHT};
    Vector2 v = {ball_p.x, ball_p.y};
    if (CheckCollisionCircleRec(v, BALL_RADIUS, r)) {
      ball_v.x *= -1;
    }
  });
}

static void BeginRendering(flecs::iter &it) {
  (void)it;
  BeginDrawing();
  ClearBackground(BLACK);
}

static void EndRendering(flecs::iter &it) {
  (void)it;
  DrawFPS(WINDOW_WIDTH - 100, 20);
  EndDrawing();
}

static void RenderBackground(flecs::iter &it) {
  (void)it;
  DrawLine(WINDOW_WIDTH / 2, 0, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 4, WHITE);
  DrawLine(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 4 * 3, WINDOW_WIDTH / 2,
           WINDOW_HEIGHT, WHITE);
  DrawCircleLines(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, WINDOW_HEIGHT / 4.0,
                  WHITE);
}

static void RenderBall(const Position &p) {
  DrawCircle(static_cast<int>(p.x), static_cast<int>(p.y), BALL_RADIUS, WHITE);
}

static void RenderPaddle(const Position &p) {
  DrawRectangle(static_cast<int>(p.x), static_cast<int>(p.y), PADDLE_WIDTH,
                PADDLE_HEIGHT, WHITE);
}

void setup_pong(flecs::world &world) {
  // Definitions
  world.component<Position>().member<float>("x").member<float>("y");
  world.component<Velocity>().member<float>("x").member<float>("y");
  world.component<Player>()
      .member(flecs::I32, "up_key")
      .member(flecs::I32, "down_key");

  // Game objects
  {
    world.entity("obj.Ball")
        .set<Position>({WINDOW_WIDTH / 2.0, WINDOW_HEIGHT / 2.0})
        .set<Velocity>({7 * 60, 7 * 60})
        .add<Ball>();
    world.entity("obj.Player")
        .set<Position>({PADDLE_GAP, WINDOW_HEIGHT / 2.0 - PADDLE_HEIGHT / 2.0})
        .set<Velocity>({0, 5 * 60})
        .set<Player>({KeyboardKey::KEY_W, KeyboardKey::KEY_S})
        .add<Paddle>();
    world.entity("obj.Cpu")
        .set<Position>({WINDOW_WIDTH - PADDLE_WIDTH - PADDLE_GAP,
                        WINDOW_HEIGHT / 2.0 - PADDLE_HEIGHT / 2.0})
        .set<Velocity>({0, 5 * 60})
        .add<Cpu>()
        .add<Paddle>();
  }

  // Systems
  {
    flecs::entity PreRendering = world.entity("PreRendering")
                                     .add(flecs::Phase)
                                     .depends_on(flecs::OnStore);
    flecs::entity OnRendering =
        world.entity("OnRendering").add(flecs::Phase).depends_on(PreRendering);
    flecs::entity PostRendering =
        world.entity("PostRendering").add(flecs::Phase).depends_on(OnRendering);

    world.system<Position, const Velocity, const Player>("MovePlayer")
        .kind(flecs::OnUpdate)
        .each(MovePlayer);
    world.system<Position, Velocity>("MoveBall")
        .with<Ball>()
        .kind(flecs::OnUpdate)
        .each(MoveBall);
    world.system<Position, const Velocity>("MoveCpu")
        .with<Cpu>()
        .kind(flecs::OnUpdate)
        .each(MoveCpu);

    world.system<const Position, Velocity>("CheckCollisions")
        .with<Ball>()
        .kind(flecs::OnValidate)
        .each(CheckCollisions);

    world.system("CheckPause").kind(flecs::PreUpdate).run(CheckPause);
    world.system("BeginRendering").kind(PreRendering).run(BeginRendering);
    world.system("RenderBackground").kind(OnRendering).run(RenderBackground);
    world.system<const Position>("RenderBall")
        .with<Ball>()
        .kind(OnRendering)
        .each(RenderBall);
    world.system<const Position>("RenderPaddle")
        .with<Paddle>()
        .kind(OnRendering)
        .each(RenderPaddle);
    world.system("EndRendering").kind(PostRendering).run(EndRendering);
  }
}
