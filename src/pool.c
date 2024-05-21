#include "pool.h"
#include <math.h>
#include <stdint.h>

#ifndef NO_THREADS
#include <pthread.h>
#endif

#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

// TODO add brute_forcing for
// add threading to full game brute_forcing
// the thread with the least moves wins

void add_ball_path_point(Ball *ball) {
  if (ball->path_count == PATH_MAX - 1) {
    return;
  }
  // printf("current index being saved in path is %d\n", ball->path_count);
  ball->path[ball->path_count++] = ball->position;
}

void init_balls(Ball *balls) {

  Vector2 rack_pos = (Vector2){470.0, 450.0};

  balls[0].type = CUE_BALL;
  balls[0].color = WHITE;
  balls[0].number = -1;
  balls[0].position = (Vector2){1230.0, 460.0};
  balls[0].pocketed = false;
  balls[0].velocity.x = -200;
  balls[0].velocity.y = -2;
  balls[0].path_count = 0;
  add_ball_path_point(&balls[0]);

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
    balls[i].path_count = 0;
    add_ball_path_point(&balls[i]);
  }
  TraceLog(LOG_INFO, "Initialized the balls");
}
void draw_ball(Ball ball);
void draw_balls(Ball balls[NUM_BALLS]);
void draw_pockets_debug();

void apply_friction_to_ball(Ball *ball) {
  // need to get vector or something
  // normalize vector and subtract negative friction
  ball->velocity.x *= FRICTION;
  ball->velocity.y *= FRICTION;

  if (sqrtf(ball->velocity.x * ball->velocity.x +
            ball->velocity.y * ball->velocity.y) < min_velocity) {
    ball->velocity.x = 0;
    ball->velocity.y = 0;
  }
  if (ball->velocity.y == 0 && ball->velocity.x == 0) {
    add_ball_path_point(ball);
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

  // need to go through all of the walls and see if
  // line intersects circle

  if (ball->position.x < 0 + TableBorder ||
      ball->position.x > ScreenWidth - TableBorder) {
    for (int i = 0;
         i < sizeof(pocket_ignore_walls) / sizeof(pocket_ignore_walls[0]);
         i++) {
      if (CheckCollisionPointCircle(ball->position, pocket_ignore_walls[i],
                                    POCKET_IGNORE_RADIUS)) {
        // is inside ignore region
        return;
      }
    }
    ball->velocity.x *= -1;
    collided_with_wall = true;
  }

  if (ball->position.y < 0 + TableBorder ||
      ball->position.y > ScreenHeight - TableBorder) {
    for (int i = 0;
         i < sizeof(pocket_ignore_walls) / sizeof(pocket_ignore_walls[0]);
         i++) {
      if (CheckCollisionPointCircle(ball->position, pocket_ignore_walls[i],
                                    POCKET_IGNORE_RADIUS)) {
        // is inside ignore region
        return;
      }
    }
    ball->velocity.y *= -1;
    collided_with_wall = true;
  }

  if (collided_with_wall) {
    ball->position.x =
        Clamp(ball->position.x, TableBorder, ScreenWidth - TableBorder);

    ball->position.y =
        Clamp(ball->position.y, TableBorder, ScreenHeight - TableBorder);
    add_ball_path_point(ball);
  }
}

void step_physics_sim(Ball *balls, int num_balls, uint32_t cur_step_count) {
  // check if balls hit each other

  for (int i = 0; i < num_balls; i++) {
    balls[i].position.x += balls[i].velocity.x;
    balls[i].position.y += balls[i].velocity.y;
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
        add_ball_path_point(&balls[i]);
        add_ball_path_point(&balls[j]);
      }
    }
  }
  // handle pockets
  for (int i = 0; i < num_balls; i++) {
    for (int j = 0; j < 6; j++) {
      bool pocketed = CheckCollisionPointCircle(balls[i].position,
                                                pocket_vecs[j], POCKET_RADIUS);
      if (pocketed && !balls[i].pocketed) {
        balls[i].pocketed = true;
        balls[i].step_count_pocketed = cur_step_count;
        balls[i].velocity = (Vector2){0, 0};
        balls[i].collision_handled = true;
        add_ball_path_point(&balls[i]);
      }
    }
  }

  for (int i = 0; i < num_balls; i++) {
    //   // try to friction the vels
    apply_friction_to_ball(&balls[i]);
  }
}

bool is_sim_at_rest(Ball *balls) {
  for (int i = 0; i < NUM_BALLS; i++) {
    if (balls[i].velocity.x != 0 || balls[i].velocity.y != 0) {
      return false;
    }
  }
  return true;
}

void benchmark_physics_sim() {

  Ball balls[NUM_BALLS] = {0};
  init_balls(&balls[0]);
  size_t step_count = 0;
  clock_t start_time = clock(); // Start timing

  // Duration for the benchmark, in seconds
  //  double benchmark_duration = 0.25; // Run the benchmark for 1 second

  // Calculate the end time based on the desired duration
  // Duration for the benchmark, in seconds
  double benchmark_duration = 1.0; // Run the benchmark for 1 second

  // Calculate the end time based on the desired duration
  clock_t end_time =
      start_time + (clock_t)(benchmark_duration * CLOCKS_PER_SEC);
  size_t sim_count = 0;
  while (clock() < end_time) {
    while (!is_sim_at_rest(&balls[0])) {
      step_physics_sim(&balls[0], NUM_BALLS, step_count);
      step_count++;
    }
    init_balls(&balls[0]);
    sim_count++;
  }

  clock_t finish_time = clock(); // End timing
  double elapsed_time = (double)(finish_time - start_time) / CLOCKS_PER_SEC;

  printf("Completed %zu physics steps and %zu sims in %.2f seconds.\n",
         step_count, sim_count, elapsed_time);
  printf("Steps per second: %.2f\n", step_count / elapsed_time);
}

uint32_t count_balls_pocketed(Ball *balls) {
  uint32_t pocket_count = 0;
  for (int i = 0; i < NUM_BALLS; i++) {
    if (balls[i].pocketed) {
      pocket_count++;
    }
  }
  return pocket_count;
}

bool is_cue_pocketed(Ball *balls) { return balls[0].pocketed; }

Vector2 random_velocity_in_degree_range(int min_degree, int max_degree) {
  // Generate a random degree between 0 and 360
  float angle_degrees = GetRandomValue(min_degree, max_degree);
  // Convert angle to radians
  float angle_radians = angle_degrees * (M_PI / 180.0f);

  // Generate a random power between 10 and 200
  float power = GetRandomValue(10, 200);

  // Convert polar coordinates (angle, power) to Cartesian coordinates (vx,
  // vy)
  Vector2 cur_velocity = {
      cos(angle_radians) * power, // vx
      sin(angle_radians) * power  // vy
  };
  return cur_velocity;
}

// returns optimal velocity for cue ball
Vector2 brute_force(int num_sims) {
  printf("Brute forcing optimal shot from %d random shots\n", num_sims);
  Ball balls[NUM_BALLS] = {0};
  Vector2 cur_best_velocity = {0, 0};
  uint32_t best_balls_pocketed = 0;

  struct timeval timecheck;

  gettimeofday(&timecheck, NULL);
  long start = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;

  SetTraceLogLevel(LOG_FATAL);
  init_balls(&balls[0]);
  for (int i = 0; i < num_sims; i++) {
    Vector2 cur_velocity = random_velocity_in_degree_range(160, 200);
    init_balls(&balls[0]);
    balls[0].velocity = cur_velocity;
    size_t step_count = 0;
    while (!is_sim_at_rest(&balls[0]) && step_count < 40000) {
      step_physics_sim(&balls[0], NUM_BALLS, step_count);
      step_count++;
    }
    uint32_t num_pocketed = count_balls_pocketed(&balls[0]);
    if (!is_cue_pocketed(&balls[0]) && num_pocketed > best_balls_pocketed) {
      cur_best_velocity = cur_velocity;
      best_balls_pocketed = num_pocketed;
    }
    if (i % 1000 == 0) {
      printf("currently at sim_count %d \n", i);
    }
  }
  SetTraceLogLevel(LOG_INFO);
  gettimeofday(&timecheck, NULL);
  long end = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;

  long elapsed_time = end - start;

  printf("Completed %d games in %ld milliseconds.\n", num_sims, elapsed_time);
  printf("Sims per second: %.2f\n",
         (float)num_sims / ((float)elapsed_time / 1000));

  printf("Best num pocketed was %d\n", best_balls_pocketed);
  return cur_best_velocity;
}

uint32_t sim_balls(Ball *balls, Vector2 cue_ball_vel, uint32_t sim_step_count) {
  balls[0].velocity = cue_ball_vel;
  size_t step_count = sim_step_count;
  while (!is_sim_at_rest(balls) && (step_count - sim_step_count) < 40000 &&
         !is_cue_pocketed(balls)) {
    step_physics_sim(balls, NUM_BALLS, step_count);
    step_count++;
  }
  return step_count;
}

void clear_ball_paths(Ball *balls) {

  for (int i = 0; i < NUM_BALLS; i++) {
    balls[i].path_count = 0;
  }
}

bool is_eight_ball_valid(Ball *balls) {
  // returns true if not pocketed
  // or was the last ball pocketed
  if (balls[8].pocketed) {
    uint32_t balls_left = NUM_BALLS - count_balls_pocketed(balls) - 1;
    if (balls_left) {
      return false;
    } else {
      // check that the pocketed step count is greatest for 8 ball
      //
      uint32_t eight_ball_pocket_step_count = balls[8].step_count_pocketed;
      for (int i = 1; i < NUM_BALLS; i++) {
        if (i == 8) {
          continue;
        }
        if (balls[i].step_count_pocketed > eight_ball_pocket_step_count) {
          return false;
        }
      }
    }
  }
  return true;
}

MoveList find_perfect_game() {
  const int MAX_MOVES = 30;
  Vector2 *velocities = malloc(sizeof(Vector2) * MAX_MOVES);
  MoveList moves = {
      .length = 0, .capacity = MAX_MOVES, .velocities = velocities};
  Ball balls[NUM_BALLS] = {0};
  Ball copyma_balls[NUM_BALLS] = {0};
  init_balls(balls);

  const int move_sim_amount = 50000;
  uint32_t sim_step_count = 0;
  for (int i = 0; i < MAX_MOVES; i++) {
    uint32_t cur_move_least_balls_left = UINT32_MAX;

    for (int j = 0; j < move_sim_amount; j++) {
      // remember the balls
      memcpy(&copyma_balls, &balls, sizeof(balls));

      Vector2 rand_vel = random_velocity_in_degree_range(0, 360);
      sim_balls(balls, rand_vel, sim_step_count);

      uint32_t balls_left = NUM_BALLS - count_balls_pocketed(balls) - 1;
      if (!is_cue_pocketed(balls) && balls_left < cur_move_least_balls_left &&
          is_eight_ball_valid(balls)) {
        // this is the vel to remember
        moves.velocities[moves.length] = rand_vel;
        cur_move_least_balls_left = balls_left;
      }
      memcpy(&balls, &copyma_balls, sizeof(balls));
    }
    moves.length++;
    sim_step_count =
        sim_balls(balls, moves.velocities[moves.length - 1], sim_step_count);

    printf("On move %d with %d balls pocketed, step_count = %d \n", i,
           count_balls_pocketed(balls), sim_step_count);
    if (!is_cue_pocketed(balls) && count_balls_pocketed(balls) == 15) {
      printf("Finished game in %zu moves!\n", moves.length);
      for (int i = 0; i < moves.length; i++) {
        printf("Vel %d is (%f,%f)\n", i, moves.velocities[i].x,
               moves.velocities[i].y);
      }
      for (int i = 1; i < NUM_BALLS; i++) {
        printf("%d ball pocketed at step count %d\n", i,
               balls[i].step_count_pocketed);
      }
      break;
    }
  }

  return moves;
}

// This find perfect game will simulate thousands of
// full games
MoveList find_perfect_game_2() {

  const int MAX_MOVES = 30;
  Vector2 *velocities = malloc(sizeof(Vector2) * MAX_MOVES);
  Vector2 *cur_best_vels = malloc(sizeof(Vector2) * MAX_MOVES);
  MoveList cur_best_moves = {
      .length = 0, .capacity = MAX_MOVES, .velocities = cur_best_vels};
  MoveList moves = {
      .length = 0, .capacity = MAX_MOVES, .velocities = velocities};
  uint32_t cur_best_moves_to_win = 30;

  Ball balls[NUM_BALLS] = {0};
  Ball copyma_balls[NUM_BALLS] = {0};
  init_balls(balls);

  const int game_sim_amount = 500000;
  uint32_t sim_step_count = 0;

  memcpy(&copyma_balls, &balls, sizeof(balls));
  for (int i = 0; i < game_sim_amount; i++) {

    // printf("simming game %d\n", i);
    if (i % 10000 == 0) {

      printf("simming game %d\n", i);
    }
    moves.length = 0;
    sim_step_count = 0;

    memcpy(&balls, &copyma_balls, sizeof(balls));
    for (int j = 0; j < MAX_MOVES; j++) {
      // remember the balls

      Vector2 rand_vel = random_velocity_in_degree_range(0, 360);
      sim_step_count = sim_balls(balls, rand_vel, sim_step_count);
      if (is_cue_pocketed(balls) ||
          (j > 1 && count_balls_pocketed(balls) < 1)) {
        break;
      }

      moves.velocities[moves.length] = rand_vel;
      moves.length++;
    }
    if (!is_cue_pocketed(balls) && count_balls_pocketed(balls) > 3) {
      printf("scored %d balls\n", count_balls_pocketed(balls));
    }

    if (!is_cue_pocketed(balls) && is_eight_ball_valid(balls) &&
        moves.length < cur_best_moves_to_win &&
        count_balls_pocketed(balls) == 15) {
      // this is the vel to remember
      printf("recorded completed game with %zu moves\n", moves.length);
      memcpy(cur_best_moves.velocities, velocities,
             sizeof(Vector2) * MAX_MOVES);
      cur_best_moves.length = moves.length;
    }
  }

  return moves;
}

#ifndef NO_THREADS
// Thread function argument structure
typedef struct {
  int start_sim;
  int end_sim;
  Vector2 best_velocity;
  uint32_t best_balls_pocketed;
} ThreadArg;

// Thread function for POSIX threads
void *thread_func(void *arg) {
  ThreadArg *thread_arg = (ThreadArg *)arg;
  Ball balls[NUM_BALLS] = {0};
  SetRandomSeed(time(NULL));
  for (int i = thread_arg->start_sim; i < thread_arg->end_sim; i++) {

    Vector2 cur_velocity = random_velocity_in_degree_range(160, 200);
    init_balls(balls);
    sim_balls(balls, cur_velocity, 0);
    uint32_t num_pocketed = count_balls_pocketed(balls);
    if (!is_cue_pocketed(balls) &&
        num_pocketed > thread_arg->best_balls_pocketed) {
      thread_arg->best_velocity = cur_velocity;
      thread_arg->best_balls_pocketed = num_pocketed;
    }
  }
  return NULL;
}

#define NUM_THREADS 16

Vector2 brute_force_threaded(int sim_count) {
  printf("Brute forcing %d shots for best with %d threads\n", sim_count,
         NUM_THREADS);

  struct timeval timecheck;

  gettimeofday(&timecheck, NULL);
  long start = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;

  int sims_per_thread = sim_count / NUM_THREADS;
  printf("INFO: Sims per thread = %d!\n", sims_per_thread);
  pthread_t threads[NUM_THREADS];
  ThreadArg thread_args[NUM_THREADS];
  Vector2 cur_best_velocity = {0, 0};
  uint32_t best_balls_pocketed = 0;

  SetTraceLogLevel(LOG_FATAL);
  // Create threads
  for (int i = 0; i < NUM_THREADS; i++) {
    thread_args[i].start_sim = i * sims_per_thread;
    thread_args[i].end_sim = (i + 1) * sims_per_thread;
    thread_args[i].best_balls_pocketed = 0;
    thread_args[i].best_velocity = (Vector2){0, 0};
    if (pthread_create(&threads[i], NULL, thread_func, &thread_args[i]) != 0) {
      printf("Failed to create thread\n");
      return (Vector2){0, 0}; // Error handling
    }
  }

  // Join threads
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
    // Aggregate results
    if (thread_args[i].best_balls_pocketed > best_balls_pocketed) {
      best_balls_pocketed = thread_args[i].best_balls_pocketed;
      cur_best_velocity = thread_args[i].best_velocity;
    }
  }
  SetTraceLogLevel(LOG_INFO);
  gettimeofday(&timecheck, NULL);
  long end = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;

  long elapsed_time = end - start;

  printf("Completed %d games in %ld milliseconds.\n", sim_count, elapsed_time);
  printf("Sims per second: %.2f\n",
         (float)sim_count / ((float)elapsed_time / 1000));

  printf("Best num pocketed was %d\n", best_balls_pocketed);
  return cur_best_velocity;
}
#endif
