#pragma once
#include <windows.h>
#include <stdbool.h>

#define GRID_SIZE 20
#define GRID_W 32
#define GRID_H 24
#define MAX_SNAKE_LEN 500
#define FIXED_STEP 0.15f // Фиксированный шаг логики: 150 мс

typedef enum { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT } Direction;
typedef struct { int x, y; } Point;

typedef struct {
    Point snake[MAX_SNAKE_LEN];
    int length;
    Direction dir;
    Direction next_dir;
    Point food;
    bool game_over;
    bool paused;
    int score;
    float update_timer;
} GameState;


void core_init(GameState* state);
void core_update(GameState* state, float dt);
void core_handle_input(GameState* state, WPARAM key);
void core_restart(GameState* state);
