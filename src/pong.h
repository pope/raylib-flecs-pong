#pragma once

#include <flecs.h>
#include <raylib.h>
#include <stdbool.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

typedef Vector2 Position;
typedef Vector2 Velocity;

extern ECS_COMPONENT_DECLARE (Position);
extern ECS_COMPONENT_DECLARE (Velocity);

extern ECS_TAG_DECLARE (Ball);

void setup_world (void);
void cleanup_world (void);
bool run_game_loop (void);
