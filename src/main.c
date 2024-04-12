#include <raylib.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  InitWindow(600, 600, "Pool Sim");
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RED);
    EndDrawing();
  }
  return EXIT_SUCCESS;
}
