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

void draw_borders() {
  for (int i = 0; i < sizeof(border_collisions) / sizeof(border_collisions[0]);
       i++) {
    if (border_collisions[i].is_horiz) {
      DrawLineV(border_collisions[i].start, border_collisions[i].end, BLUE);
    } else {
      DrawLineV(border_collisions[i].start, border_collisions[i].end, RED);
    }
  }
  for (int i = 0;
       i < sizeof(pocket_ignore_walls) / sizeof(pocket_ignore_walls[0]); i++) {
    DrawCircleV(pocket_ignore_walls[i], POCKET_IGNORE_RADIUS,
                (Color){.r = 0, .g = 0, .b = 100, .a = 150});
  }
}

void trace_paths(Ball *balls) {

  for (int i = 0; i < NUM_BALLS; i++) {
    // draw the path for a ball
    if (balls[i].path_count < PATH_MAX - 1) {
      balls[i].path[balls[i].path_count + 1] = balls[i].position;
      DrawLineStrip(balls[i].path, balls[i].path_count + 1, balls[i].color);
    } else {

      DrawLineStrip(balls[i].path, balls[i].path_count, balls[i].color);
    }
  }
}

int main(int argc, char *argv[]) {
  SetTargetFPS(60);
  InitWindow(1700, 900, "Pool Sim");

  Ball balls[NUM_BALLS] = {0};
  init_balls(&balls[0]);
  // benchmark_physics_sim();
  Vector2 optimal_velocity = brute_force_threaded();
  balls[0].velocity = optimal_velocity;

  Texture2D table_texture = LoadTexture("assets/pool_table.png");

  while (!WindowShouldClose()) {

    if (IsKeyPressed(KEY_SPACE)) {
      init_balls(&balls[0]);
      balls[0].velocity = optimal_velocity;
    }

    BeginDrawing();
    // DrawPoolTable
    draw_pool_table(table_texture);
    draw_pockets_debug();
#ifndef DONT_DRAW_PATHS
    trace_paths(&balls[0]);
#endif
    draw_balls(balls);
    draw_borders();

    Vector2 mouse_pos = GetMousePosition();
    const char *mouse_pos_str =
        TextFormat("Pos = %d,%d\n", (int)mouse_pos.x, (int)mouse_pos.y);
    DrawText(mouse_pos_str, 0, 0, 30, BLACK);

    EndDrawing();
    step_physics_sim(&balls[0], sizeof(balls) / sizeof(balls[0]));
    if (is_sim_at_rest(&balls[0])) {
      // randomize veloicity of cue ball and shoot again
      init_balls(&balls[0]);
      balls[0].velocity =
          (Vector2){GetRandomValue(-60, 60), GetRandomValue(-60, 60)};
    }
  }
  return EXIT_SUCCESS;
}
