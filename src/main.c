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
  InitWindow(1700, 900, "Pool Sim");

  Ball balls[16] = {0};
  init_balls(&balls[0]);

  Texture2D table_texture = LoadTexture("assets/pool_table.png");

  while (!WindowShouldClose()) {
    BeginDrawing();
    // DrawPoolTable
    ClearBackground(RED);
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
    Vector2 mouse_pos = GetMousePosition();
    const char *mouse_pos_str =
        TextFormat("Pos = %d,%d\n", (int)mouse_pos.x, (int)mouse_pos.y);
    DrawText(mouse_pos_str, 0, 0, 30, BLUE);

    EndDrawing();
  }
  return EXIT_SUCCESS;
}
