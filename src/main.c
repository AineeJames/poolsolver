#include <math.h>
#include <raylib.h>
#include <stdlib.h>

#define BALL_RADIUS 25
#define BALL_PADDING 3
#define FRICTION 0.001
#define POCKET_RADIUS 50
#define SPEED 1

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
  balls[0].position = (Vector2){1230.0, 450.0};
  balls[0].pocketed = false;

  int rack_col = 1;
  int ball_idx = 0;
  for (int i = 1; i < 16; i++) {
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
        (rack_col * BALL_PADDING) + GetRandomValue(0, BALL_PADDING / 2);
    float ball_y = rack_pos.y - (rack_col * BALL_RADIUS) +
                   (ball_idx * (BALL_RADIUS * 2)) + (ball_idx * BALL_PADDING) -
                   GetRandomValue(0, BALL_PADDING / 2);
    balls[i].position = (Vector2){ball_x, ball_y};
    ball_idx += 1;
    if (ball_idx == rack_col) {
      rack_col++;
      ball_idx = 0;
    }
    balls[i].pocketed = false;
  }
  TraceLog(LOG_INFO, "Initialized the balls");
}
void draw_ball(Ball ball);
void draw_balls(Ball balls[16]);
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
  if (ball->velocity.x > 0) {
    if (ball->velocity.x < FRICTION) {
      ball->velocity.x = 0;
    } else {
      ball->velocity.x -= FRICTION;
    }
  } else {
    if (ball->velocity.y > -FRICTION) {
      ball->velocity.y = 0;
    } else {
      ball->velocity.y += FRICTION;
    }
  }
}

void update_ball_velocities(Ball *ball1, Ball *ball2) {
  Vector2 pos_delta = {0};
  pos_delta.x = fabs(ball1->position.x - ball2->position.x);
  pos_delta.y = fabs(ball1->position.y - ball2->position.y);

  double distance = sqrtf(pow(ball1->position.x - ball2->position.x, 2) +
                          pow(ball1->position.y - ball2->position.y, 2));

  double nx = (ball1->position.x - ball2->position.y) / distance;
  double ny = (ball1->position.y - ball2->position.y) / distance;

  double p = (ball1->velocity.x * nx + ball1->velocity.y * ny -
              ball2->velocity.x * nx - ball2->velocity.y * ny);
  ball1->velocity.x = ball1->velocity.x - p * nx;
  ball1->velocity.y = ball1->velocity.y - p * ny;
  ball1->velocity.x = ball1->velocity.x - p * nx;
  ball1->velocity.y = ball1->velocity.y - p * ny;
}

void handle_ball_hit_wall(Ball *ball) {}

void step_physics_sim(Ball *balls, int num_balls) {
  // check if balls hit each other
  for (int i = 0; i < num_balls; i++) {
    for (int j = 0; j < num_balls; j++) {
      if (i == j) {
        continue;
      }
      // bool is_ball_hitting = CheckCollisionCircles(
      //     balls[i].position, BALL_RADIUS, balls[j].position, BALL_RADIUS);
      if (balls[i].collision_handled == false &&
          balls[i].collision_handled == false) {
        // bool is_ball_hitting =
        if (CheckCollisionCircles(balls[i].position, BALL_RADIUS,
                                  balls[j].position, BALL_RADIUS)) {

          update_ball_velocities(&balls[i], &balls[j]);
          balls[i].collision_handled = true;
          balls[j].collision_handled = true;
        }
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
        balls[i].collision_handled = false;
      }
    }
    handle_ball_hit_wall(&balls[i]);
  }

  for (int i = 0; i < num_balls; i++) {
    // try to friction the vels
    apply_friction_to_ball(&balls[i]);
    balls[i].position.x += balls[i].velocity.x * SPEED;
    balls[i].position.y += balls[i].velocity.y * SPEED;
  }
}

int main(int argc, char *argv[]) {
  InitWindow(1700, 900, "Pool Sim");

  Ball balls[16] = {0};
  init_balls(&balls[0]);
  balls[0].velocity.x = -20;

  Texture2D table_texture = LoadTexture("assets/pool_table.png");

  while (!WindowShouldClose()) {
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
    step_physics_sim(&balls[0], sizeof(balls) / sizeof(balls[0]));
  }
  return EXIT_SUCCESS;
}

void draw_pockets_debug() {
  for (int i = 0; i < 6; i++) {
    DrawCircleV(pocket_vecs[i], POCKET_RADIUS, Fade(RED, 0.3));
  }
}

void draw_balls(Ball balls[16]) {
  for (int i = 0; i < 16; i++) {
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
