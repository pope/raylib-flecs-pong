#include <raylib.h>
#include <stdlib.h>

#include "pong.h"

int
main (void)
{
  SetConfigFlags (FLAG_MSAA_4X_HINT);
  InitWindow (WINDOW_WIDTH, WINDOW_HEIGHT, "Pope Pong");
  SetTargetFPS (60);

  setup_world ();

  while (!WindowShouldClose () && run_game_loop ())
    {
    }

  cleanup_world ();
  CloseWindow ();

  return EXIT_SUCCESS;
}
