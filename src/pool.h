#ifndef POOL_H
#define POOL_H
#include <raylib.h>
#include <stddef.h>
#include <stdint.h>

#define BALL_RADIUS 25
#define BALL_PADDING 3
#define FRICTION 0.98
#define POCKET_RADIUS 50
#define POCKET_IGNORE_RADIUS 40
#define SPEED 0.03
#define NUM_BALLS 16

#define PATH_MAX 100

static const Vector2 pocket_vecs[6] = {
    {80.0, 73.0},    // top left
    {80.0, 825.0},   // btm left
    {850.0, 39.0},   // top middle
    {850.0, 860.0},  // btm middle
    {1620.0, 73.0},  // top right
    {1620.0, 825.0}, // btm right
};

static const Vector2 pocket_ignore_walls[6] = {
    {115.0, 108.0},  // top left
    {115.0, 789.0},  // btm left
    {850.0, 90.0},   // top middle
    {850.0, 810.0},  // btm middle
    {1585.0, 107.0}, // top right
    {1590.0, 788.0}, // btm right
};

typedef struct Line {
  Vector2 start;
  Vector2 end;
  bool is_horiz;
} Line;

static const Line border_collisions[6] = {
    {{150, 810}, {808, 810}, true},    // bottom left
    {{895, 810}, {1550, 810}, true},   // bottom right
    {{95, 145}, {95, 756}, false},     // left side
    {{1610, 146}, {1610, 756}, false}, // right side
    {{150, 90}, {808, 90}, true},      // top left
    {{895, 90}, {1550, 90}, true},     // top right
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
  uint32_t step_count_pocketed;
  Vector2 path[PATH_MAX];
  uint32_t path_count;
} Ball;

typedef struct {
  size_t length;
  size_t capacity;
  Vector2 *velocities;
} MoveList;

void init_balls(Ball *balls);
void benchmark_physics_sim();
void step_physics_sim(Ball *balls, int num_balls, uint32_t cur_step_count);
bool is_sim_at_rest(Ball *balls);
void clear_ball_paths(Ball *balls);

MoveList find_perfect_game();
MoveList find_perfect_game_2();
#ifndef NO_THREADS
Vector2 brute_force_threaded(int num_sims);
#endif
Vector2 brute_force(int num_sims);

#endif

Vector2 random_velocity_in_degree_range(int min_degree, int max_degree);
