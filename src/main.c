#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BALL_RADIUS 25
#define BALL_PADDING 3
#define FRICTION 0.98
#define POCKET_RADIUS 50
#define SPEED 0.03
#define NUM_BALLS 16

const float min_velocity = 0.2;
const int ScreenWidth = 1700;
const int ScreenHeight = 900;

const Vector2 pocket_vecs[6] = {
    {80.0, 73.0},    // top left
    {80.0, 825.0},   // btm left
    {850.0, 39.0},   // top middle
    {850.0, 860.0},  // btm middle
    {1620.0, 73.0},  // top right
    {1620.0, 825.0}, // btm right
};

typedef enum {
  REG_BALL,
  CUE_BALL,
} BallType;

typedef struct Ball {
  Vector2 position;
  BallType type;
  Color color;
  int number;
  Vector2 velocity;
  bool collision_handled;
  bool pocketed;
} Ball;

void init_balls(Ball *balls) {

  Vector2 rack_pos = (Vector2){470.0, 450.0};

  balls[0].type = CUE_BALL;
  balls[0].color = WHITE;
  balls[0].number = -1;
  balls[0].position = (Vector2){1230.0, 470.0};
  balls[0].pocketed = false;
  balls[0].velocity.x = -2000;
  balls[0].velocity.y = -2;

  int rack_col = 1;
  int ball_idx = 0;
  for (int i = 1; i < NUM_BALLS; i++) {
    balls[i].type = REG_BALL;
    balls[i].color = i == 8 ? BLACK
                            : (Color){.r = GetRandomValue(50, 255),
                                      .g = GetRandomValue(50, 255),
                                      .b = GetRandomValue(50, 0),
                                      .a = 255};
    balls[i].number = i;

    float ball_x =
        rack_pos.x -
        ((rack_col - 1) * sqrt(pow(BALL_RADIUS * 2, 2) - pow(BALL_RADIUS, 2))) -
        (rack_col * BALL_PADDING); // + GetRandomValue(0, BALL_PADDING / 2);
    float ball_y = rack_pos.y - (rack_col * BALL_RADIUS) +
                   (ball_idx * (BALL_RADIUS * 2)) +
                   (ball_idx * BALL_PADDING); //-
    // GetRandomValue(0, BALL_PADDING / 2);
    balls[i].position = (Vector2){ball_x, ball_y};
    ball_idx += 1;
    if (ball_idx == rack_col) {
      rack_col++;
      ball_idx = 0;
    }
    balls[i].pocketed = false;
    balls[i].velocity = (Vector2){0};
  }
  TraceLog(LOG_INFO, "Initialized the balls");
}
void draw_ball(Ball ball);
void draw_balls(Ball balls[NUM_BALLS]);
void draw_pockets_debug();

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

void apply_friction_to_ball(Ball *ball) {
  // need to get vector or something
  // normalize vector and subtract negative friction
  ball->velocity.x *= FRICTION;
  ball->velocity.y *= FRICTION;

  if (fabs(ball->velocity.x) < min_velocity) {
    ball->velocity.x = 0;
  }
  if (fabs(ball->velocity.y) < min_velocity) {
    ball->velocity.y = 0;
  }
}

void adjust_ball_position(Ball *ball1, Ball *ball2) {

  //  double dx = ball1->position.x - ball2->position.x;
  //  double dy = ball1->position.y - ball2->position.y;
  float dx = ball2->position.x - ball1->position.x;
  float dy = ball2->position.y - ball1->position.y;
  float angle = atan2(dy, dx);
  float center_x = ball1->position.x + 0.5 * dx;
  float center_y = ball1->position.y + 0.5 * dy;
  float radius = BALL_RADIUS;
  ball1->position.x = center_x - (cos(angle) * radius);
  ball1->position.y = center_y - (sin(angle) * radius);
  ball2->position.x = center_x + (cos(angle) * radius);
  ball2->position.y = center_y + (sin(angle) * radius);
}

void update_ball_velocities(Ball *ball1, Ball *ball2) {
  float distance = sqrtf(pow(ball1->position.x - ball2->position.x, 2) +
                         pow(ball1->position.y - ball2->position.y, 2));

  float nx = (ball2->position.x - ball1->position.x) / distance;
  float ny = (ball2->position.y - ball1->position.y) / distance;

  float p = (ball1->velocity.x * nx + ball1->velocity.y * ny -
             ball2->velocity.x * nx - ball2->velocity.y * ny);
  ball1->velocity.x = ball1->velocity.x - p * nx;
  ball1->velocity.y = ball1->velocity.y - p * ny;
  ball2->velocity.x = ball2->velocity.x + p * nx;
  ball2->velocity.y = ball2->velocity.y + p * ny;
}

void handle_ball_hit_wall(Ball *ball) {
  bool collided_with_wall = false;

  if (ball->position.x < 0 || ball->position.x > ScreenWidth) {
    ball->velocity.x *= -1;
    collided_with_wall = true;
  }

  if (ball->position.y < 0 || ball->position.y > ScreenHeight) {
    ball->velocity.y *= -1;
    collided_with_wall = true;
  }

  if (collided_with_wall) {
    Clamp(ball->position.x, BALL_RADIUS, ScreenWidth - BALL_RADIUS);
    Clamp(ball->position.y, BALL_RADIUS, ScreenHeight - BALL_RADIUS);
  }
}

void step_physics_sim(Ball *balls, int num_balls) {
  // check if balls hit each other

  for (int i = 0; i < num_balls; i++) {
    balls[i].position.x += balls[i].velocity.x * SPEED;
    balls[i].position.y += balls[i].velocity.y * SPEED;
    handle_ball_hit_wall(&balls[i]);
  }

  for (int i = 0; i < num_balls; i++) {
    for (int j = i + 1; j < num_balls; j++) {
      if (balls[i].pocketed || balls[j].pocketed) {
        continue;
      }
      if (CheckCollisionCircles(balls[i].position, BALL_RADIUS,
                                balls[j].position, BALL_RADIUS)) {
        update_ball_velocities(&balls[i], &balls[j]);
        adjust_ball_position(&balls[i], &balls[j]);
      }
    }
  }
  // handle pockets
  for (int i = 0; i < num_balls; i++) {
    for (int j = 0; j < 6; j++) {
      bool pocketed = CheckCollisionPointCircle(balls[i].position,
                                                pocket_vecs[j], POCKET_RADIUS);
      if (pocketed) {
        balls[i].pocketed = true;
        balls[i].velocity = (Vector2){0, 0};
        balls[i].collision_handled = true;
      }
    }
  }

  for (int i = 0; i < num_balls; i++) {
    //   // try to friction the vels
    apply_friction_to_ball(&balls[i]);
  }
}

void benchmark_physics_sim() {

  Ball balls[NUM_BALLS] = {0};
  init_balls(&balls[0]);
  size_t step_count = 0;
  clock_t start_time = clock(); // Start timing

  // Duration for the benchmark, in seconds
  double benchmark_duration = 0.25; // Run the benchmark for 1 second

  // Calculate the end time based on the desired duration
  clock_t end_time =
      start_time + (clock_t)(benchmark_duration * CLOCKS_PER_SEC);

  while (clock() < end_time) {
    step_physics_sim(&balls[0], NUM_BALLS);
    step_count++;
  }

  clock_t finish_time = clock(); // End timing
  double elapsed_time = (double)(finish_time - start_time) / CLOCKS_PER_SEC;

  printf("Completed %zu physics steps in %.2f seconds.\n", step_count,
         elapsed_time);
  printf("Steps per second: %.2f\n", step_count / elapsed_time);
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

void draw_pockets_debug() {
  for (int i = 0; i < 6; i++) {
    DrawCircleV(pocket_vecs[i], POCKET_RADIUS, Fade(RED, 0.3));
  }
}

void draw_balls(Ball balls[NUM_BALLS]) {
  for (int i = 0; i < NUM_BALLS; i++) {
    if (!balls[i].pocketed)
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
