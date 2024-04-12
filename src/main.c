#include <math.h>
#include <raylib.h>
#include <stdlib.h>

#define BALL_RADIUS 25
#define BALL_PADDING 3

typedef enum {
  REG_BALL,
  CUE_BALL,
} BallType;

typedef struct Ball {
  Vector2 position;
  BallType type;
  Color color;
  int number;
} Ball;

void init_balls(Ball *balls) {

  Vector2 rack_pos = (Vector2){470.0, 450.0};

  balls[0].type = CUE_BALL;
  balls[0].color = WHITE;
  balls[0].number = -1;
  balls[0].position = (Vector2){1230.0, 450.0};

  int rack_col = 1;
  int ball_idx = 0;
  for (int i = 1; i < 16; i++) {
    balls[i].type = REG_BALL;
    balls[i].color = i == 8 ? BLACK : (Color){
      .r = GetRandomValue(50, 255),
      .g = GetRandomValue(50, 255),
      .b = GetRandomValue(50, 0),
      .a = 255
    };
    balls[i].number = i;
    
    float ball_x = rack_pos.x - (rack_col * sqrt(pow(BALL_RADIUS * 2, 2) - pow(BALL_RADIUS, 2))) - (rack_col * BALL_PADDING) + GetRandomValue(0, BALL_PADDING / 2);
    float ball_y = rack_pos.y - (rack_col * BALL_RADIUS) + (ball_idx * (BALL_RADIUS * 2)) + (ball_idx * BALL_PADDING) - GetRandomValue(0, BALL_PADDING / 2);
    balls[i].position = (Vector2){ball_x, ball_y};
    ball_idx += 1;
    if (ball_idx == rack_col) { rack_col++; ball_idx = 0; }
  }
  TraceLog(LOG_INFO, "Initialized the balls");
}
void draw_ball(Ball ball);
void draw_balls(Ball balls[16]);

int main(int argc, char *argv[]) {
  InitWindow(1700, 900, "Pool Sim");

  Ball balls[16] = {0};
  init_balls(&balls[0]);

  Texture2D table_texture = LoadTexture("assets/pool_table.png");

  while (!WindowShouldClose()) {
    BeginDrawing();
    // DrawPoolTable
    DrawTexturePro(table_texture,
                   (Rectangle){.x = 0,
                               .y = 0,
                               .width = table_texture.width,
                               .height = table_texture.height},
                   (Rectangle){.x = 0,
                               .y = 0,
                               .width = GetScreenWidth(),
                               .height = GetScreenHeight()},
                   (Vector2){0, 0}, 0, WHITE);
    draw_balls(balls);

    Vector2 mouse_pos = GetMousePosition();
    const char *mouse_pos_str =
        TextFormat("Pos = %d,%d\n", (int)mouse_pos.x, (int)mouse_pos.y);
    DrawText(mouse_pos_str, 0, 0, 30, BLUE);

    EndDrawing();
  }
  return EXIT_SUCCESS;
}

void draw_balls(Ball balls[16]) {
  for (int i = 0; i < 16; i++) {
    draw_ball(balls[i]);
  }
}

void draw_ball(Ball ball) {
  DrawCircleV(ball.position, BALL_RADIUS, ball.color);
  if (ball.number > 0) {
    DrawCircleV(ball.position, BALL_RADIUS - (BALL_RADIUS / 3.), WHITE);
    int fontSize = (BALL_RADIUS);
    const char *number_str = TextFormat("%d", ball.number);
    int strLen = MeasureText(number_str, fontSize);
    int textPositionX = ball.position.x - (strLen / 2.);
    int textPositionY = ball.position.y - (fontSize / 2.);
    DrawText(TextFormat("%d", ball.number), textPositionX, textPositionY,
             fontSize, BLACK);
  }
}
