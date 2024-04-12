#include <raylib.h>
#include <stdlib.h>

#define BALL_RADIUS 15

typedef enum {
  REG_BALL,
  CUE_BALL,
} BallType;

typedef struct {
  Vector2 position;
  BallType type;
  Color color;
  int number;
} Ball;

void init_balls(Ball *balls) { balls[0].type = CUE_BALL; }
void draw_ball(Ball ball);

int main(int argc, char *argv[]) {
  InitWindow(600, 600, "Pool Sim");

  Ball balls[16] = {0};
  init_balls(&balls[0]);

  while (!WindowShouldClose()) {
    BeginDrawing();
    // DrawPoolTable
    Ball test_ball = {
      .position = (Vector2){300, 300},
      .type = REG_BALL,
      .color = BLUE,
      .number = 1
    };
    draw_ball(test_ball);
    ClearBackground(RED);
    EndDrawing();
  }
  return EXIT_SUCCESS;
}

void draw_ball(Ball ball) {
  DrawCircleV(ball.position, BALL_RADIUS, ball.color);
  DrawCircleV(ball.position, BALL_RADIUS - (BALL_RADIUS / 3.), WHITE);
  int fontSize = (BALL_RADIUS * 1.25);
  const char* number_str = TextFormat("%d", ball.number);
  int strLen = MeasureText(number_str, fontSize);
  int textPositionX = ball.position.x - (strLen / 2.);
  int textPositionY = ball.position.y - (fontSize / 2.);
  DrawText(TextFormat("%d", ball.number), textPositionX, textPositionY, fontSize, BLACK);
}
