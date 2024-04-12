#include <raylib.h>
#include <stdlib.h>

typedef enum {
  REG_BALL,
  CUE_BALL,
} BallType;

typedef struct {
  Vector2 position;
  BallType type;
} Ball;

void init_balls(Ball *balls) { balls[0].type = CUE_BALL; }

int main(int argc, char *argv[]) {
  InitWindow(600, 600, "Pool Sim");

  Ball balls[16] = {0};
  init_balls(&balls[0]);

  while (!WindowShouldClose()) {
    BeginDrawing();
    // DrawPoolTable
    ClearBackground(RED);
    EndDrawing();
  }
  return EXIT_SUCCESS;
}
