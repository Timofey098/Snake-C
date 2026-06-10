#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

static bool is_snake_at(const GameState* state, int x, int y) {
    for (int i = 0; i < state->length; ++i) {
        if (state->snake[i].x == x && state->snake[i].y == y) return true;
    }
    return false;
}

static void place_food(GameState* state) {
    int x, y;
    do {
        x = rand() % GRID_W;
        y = rand() % GRID_H;
    } while (is_snake_at(state, x, y));
    state->food.x = x;
    state->food.y = y;
}

void core_init(GameState* state) {
    memset(state, 0, sizeof(GameState));
    state->length = 3;
    state->snake[0].x = GRID_W / 2;
    state->snake[0].y = GRID_H / 2;
    state->snake[1].x = state->snake[0].x - 1;
    state->snake[1].y = state->snake[0].y;
    state->snake[2].x = state->snake[0].x - 2;
    state->snake[2].y = state->snake[0].y;

    state->dir = DIR_RIGHT;
    state->next_dir = DIR_RIGHT;
    state->update_timer = 0.0f;
    state->game_over = false;
    state->score = 0;

    srand((unsigned int)time(NULL));
    place_food(state);
}

void core_handle_input(GameState* state, WPARAM key) {
    if (state->game_over) {
        if (key == VK_RETURN || key == VK_SPACE) core_restart(state);
        return;
    }
    if (key == 'P' || key == 'p') {
        state->paused = !state->paused;
        return;
    }
    if (state->paused) return;

    // Запрет разворота на 180 градусов
    switch (key) {
    case VK_UP:    if (state->dir != DIR_DOWN) state->next_dir = DIR_UP; break;
    case VK_DOWN:  if (state->dir != DIR_UP)   state->next_dir = DIR_DOWN; break;
    case VK_LEFT:  if (state->dir != DIR_RIGHT) state->next_dir = DIR_LEFT; break;
    case VK_RIGHT: if (state->dir != DIR_LEFT) state->next_dir = DIR_RIGHT; break;
    }
}

void core_update(GameState* state, float dt) {
    if (state->game_over || state->paused) return;

    state->update_timer += dt;
    if (state->update_timer < FIXED_STEP) return;
    state->update_timer -= FIXED_STEP;

    state->dir = state->next_dir;

    Point head = state->snake[0];
    switch (state->dir) {
    case DIR_UP:    head.y--; break;
    case DIR_DOWN:  head.y++; break;
    case DIR_LEFT:  head.x--; break;
    case DIR_RIGHT: head.x++; break;
    }

    // Коллизия со стенами
    if (head.x < 0 || head.x >= GRID_W || head.y < 0 || head.y >= GRID_H) {
        state->game_over = true;
        return;
    }
    // Коллизия с собой
    for (int i = 0; i < state->length; ++i) {
        if (head.x == state->snake[i].x && head.y == state->snake[i].y) {
            state->game_over = true;
            return;
        }
    }

    // Движение тела
    for (int i = state->length - 1; i > 0; --i) {
        state->snake[i] = state->snake[i - 1];
    }
    state->snake[0] = head;

    // Поедание еды
    if (head.x == state->food.x && head.y == state->food.y) {
        if (state->length < MAX_SNAKE_LEN) {
            state->snake[state->length] = head;
            state->length++;
        }
        state->score += 10;
        place_food(state);
    }
}

void core_restart(GameState* state) {
    core_init(state);
}