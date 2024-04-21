#ifndef POOL_H
#define POOL_H
#include <raylib.h>

#define BALL_RADIUS 25
#define BALL_PADDING 3
#define FRICTION 0.98
#define POCKET_RADIUS 50
#define SPEED 0.03
#define NUM_BALLS 16

static const Vector2 pocket_vecs[6] = {
    {80.0, 73.0},    // top left
    {80.0, 825.0},   // btm left
    {850.0, 39.0},   // top middle
    {850.0, 860.0},  // btm middle
    {1620.0, 73.0},  // top right
    {1620.0, 825.0}, // btm right
};

static const float min_velocity = 0.2;
static const int ScreenWidth = 1700;
static const int ScreenHeight = 900;
static const int TableBorder = 100;

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

void init_balls(Ball *balls);
void benchmark_physics_sim();
void step_physics_sim(Ball *balls, int num_balls);

#endif
