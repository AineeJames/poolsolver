#include "pool.h"
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

void draw_pool_table(Texture2D table_texture) {
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
}

void draw_pockets_debug() {
  for (int i = 0; i < 6; i++) {
    DrawCircleV(pocket_vecs[i], POCKET_RADIUS, Fade(RED, 0.3));
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

void draw_balls(Ball balls[NUM_BALLS]) {
  for (int i = 0; i < NUM_BALLS; i++) {
    if (!balls[i].pocketed)
      draw_ball(balls[i]);
  }
}

int main(int argc, char *argv[]) {
  SetTargetFPS(60);
  InitWindow(1700, 900, "Pool Sim");

  Ball balls[NUM_BALLS] = {0};
  init_balls(&balls[0]);
  benchmark_physics_sim();

  Texture2D table_texture = LoadTexture("assets/pool_table.png");

  int frame_cnt = 0;
  while (!WindowShouldClose()) {

    if (IsKeyPressed(KEY_SPACE)) {
      init_balls(&balls[0]);
    }

    BeginDrawing();
    // DrawPoolTable
    draw_pool_table(table_texture);
    draw_pockets_debug();
    draw_balls(balls);

    Vector2 mouse_pos = GetMousePosition();
    const char *mouse_pos_str =
        TextFormat("Pos = %d,%d\n", (int)mouse_pos.x, (int)mouse_pos.y);
    DrawText(mouse_pos_str, 0, 0, 30, BLACK);

    EndDrawing();
    if (frame_cnt % 1 == 0)
      step_physics_sim(&balls[0], sizeof(balls) / sizeof(balls[0]));
    frame_cnt++;
  }
  return EXIT_SUCCESS;
}
