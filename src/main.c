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

int main(int argc, char *argv[]) {
  InitWindow(600, 600, "Pool Sim");
  Ball balls[16] = {0};
  balls[0].type = CUE_BALL;

  while (!WindowShouldClose()) {
    BeginDrawing();
    // DrawPoolTable
    ClearBackground(RED);
    EndDrawing();
  }
  return EXIT_SUCCESS;
}
