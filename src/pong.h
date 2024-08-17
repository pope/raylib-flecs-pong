#pragma once

#include <flecs.h>
#include <raylib.h>
#include <stdbool.h>

// TODO(pope): Maybe we don't need these if we can use GetScreenWidth() and
// GetScreenHeight()
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#ifdef __cplusplus
extern "C"
{
#endif

  void setup_world (void);
  void cleanup_world (void);
  bool run_game_loop (void);

#ifdef __cplusplus
}
#endif
