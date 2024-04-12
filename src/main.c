#include <raylib.h>
#include <stdlib.h>

#define BALL_RADIUS 25
#define FRICTION 0.001

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
} Ball;

void init_balls(Ball *balls) {
  int *leaker = malloc(1000);

  leaker[0] = 1;
  balls[0].type = CUE_BALL;
  balls[0].color = WHITE;

  for (int i = 1; i < 16; i++) {
    balls[i].type = REG_BALL;
    balls[i].color = (Color){.r = GetRandomValue(0, 255),
                             .g = GetRandomValue(0, 255),
                             .b = GetRandomValue(0, 0),
                             .a = 255};
  }
  TraceLog(LOG_INFO, "Initialized the balls");
}
void draw_ball(Ball ball);

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

void update_ball_velocities(Ball *ball1, Ball *ball2) {}

void step_physics_sim(Ball *balls, int num_balls) {
  // check if balls hit each other
  for (int i = 0; i < num_balls; i++) {
    for (int j = 0; j < num_balls; j++) {
      if (i == j) {
        continue;
      }
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

      // check if center inside holes
    }
  }

  // check if a ball should fall in hole
  for (int i = 0; i < num_balls; i++) {
    // bool CheckCollisionPointCircle(balls[i].position, hole_location,
    // hole_radius);
    balls[i].collision_handled = false;

    // try to friction the vels
    apply_friction_to_ball(&balls[i]);
  }
}

int main(int argc, char *argv[]) {
  InitWindow(1700, 900, "Pool Sim");

  Ball balls[16] = {0};
  init_balls(&balls[0]);

  Texture2D table_texture = LoadTexture("assets/pool_table.png");

  while (!WindowShouldClose()) {
    BeginDrawing();
    // DrawPoolTable
    draw_pool_table(table_texture);

    Ball test_ball = {.position = (Vector2){300, 300},
                      .type = REG_BALL,
                      .color = BLUE,
                      .number = 1};
    draw_ball(test_ball);
    ClearBackground(RED);
    Vector2 mouse_pos = GetMousePosition();
    const char *mouse_pos_str =
        TextFormat("Pos = %d,%d\n", (int)mouse_pos.x, (int)mouse_pos.y);
    DrawText(mouse_pos_str, 0, 0, 30, BLUE);

    EndDrawing();
  }
  return EXIT_SUCCESS;
}

void draw_ball(Ball ball) {
  DrawCircleV(ball.position, BALL_RADIUS, ball.color);
  DrawCircleV(ball.position, BALL_RADIUS - (BALL_RADIUS / 3.), WHITE);
  int fontSize = (BALL_RADIUS * 1.25);
  const char *number_str = TextFormat("%d", ball.number);
  int strLen = MeasureText(number_str, fontSize);
  int textPositionX = ball.position.x - (strLen / 2.);
  int textPositionY = ball.position.y - (fontSize / 2.);
  DrawText(TextFormat("%d", ball.number), textPositionX, textPositionY,
           fontSize, BLACK);
}
