#include <raylib.h>
#include <stdlib.h>

typedef enum {
  REG_BALL,
  CUE_BALL,
} BallType;

typedef struct Ball {
  Vector2 position;
  BallType type;
  Color ball_color;
} Ball;

void init_balls(Ball *balls) {
  int *leaker = malloc(1000);

  leaker[0] = 1;
  balls[0].type = CUE_BALL;
  balls[0].ball_color = WHITE;

  for (int i = 1; i < 16; i++) {
    balls[i].type = REG_BALL;
    balls[i].ball_color = (Color){.r = GetRandomValue(0, 255),
                                  .g = GetRandomValue(0, 255),
                                  .b = GetRandomValue(0, 0),
                                  .a = 255};
  }
  TraceLog(LOG_INFO, "Initialized the balls");
}

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
