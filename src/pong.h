#pragma once

#include <flecs.h>
#include <stdbool.h>

// TODO(pope): Maybe we don't need these if we can use GetScreenWidth() and
// GetScreenHeight()
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#ifdef __cplusplus
extern "C"
{
#endif

  void setup_pong (ecs_world_t *world);

#ifdef __cplusplus
}
#endif
