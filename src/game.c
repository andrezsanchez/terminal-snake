#include "game.h"

const vec2i game_dim = { GAME_SIZE, GAME_SIZE };

void game_init(game_t * game) {
  if (!game) {
    return;
  }

  // Destroy any existing values.
  game_destroy(game);

  game->position = (vec2i) { GAME_SIZE / 2, GAME_SIZE / 2 };
  game->direction = (vec2i) { 0, 1 };
  game->snake = list_new();
  game->snake->free = free;
  game->end_screen = false;
  game->apple = (vec2i) { 0, 1 };
  for (int i = 0; i < 5; i += 1) {
    list_rpush(
      game->snake,
      list_node_new(
        vec2i_new(game->position.x + i, game->position.y)
      )
    );
  }
}

void game_end(game_t * game) {
  if (!game) {
    return;
  }

  game->end_screen = true;
}

void game_destroy(game_t * game) {
  if (!game) {
    return;
  }

  if (game->snake != NULL) {
    list_destroy(game->snake);
  }

  *game = (game_t) {0};
}

void game_reset_apple(game_t * game) {
  if (!game) {
    return;
  }

  do {
    game->apple.x = rand() % 30;
    game->apple.y = rand() % 30;
  } while (block_snake_collision(game->apple, game->snake));
}

void next_direction(
  vec2i * currentDirection,
  const vec2i inputDirection
) {
  if (!currentDirection) {
    return;
  }

  // Copy the direction under certain criteria.
  if (
    // Prevent a direction directly opposite the current direction.
    !(currentDirection->x == inputDirection.x && currentDirection->y == -inputDirection.y) &&
    !(currentDirection->x == -inputDirection.x && currentDirection->y == inputDirection.y) &&

    // Don't use a 0 direction.
    !(inputDirection.x == 0 && inputDirection.y == 0)
   ) {
    vec2i_copy(currentDirection, inputDirection);
  }
}

bool in_bounds(const vec2i position, const vec2i bounds) {
  if (position.x < 0) return false;
  if (position.x >= bounds.x) return false;
  if (position.y < 0) return false;
  if (position.y >= bounds.y) return false;
  return true;
}

void game_apply_direction(game_t * game, vec2i direction) {
  if (!game) {
    return;
  }

  next_direction(&(game->direction), direction);
  vec2i candidatePosition = {0};
  vec2i_add(&candidatePosition, game->position, game->direction);

  // Check whether the snake has collided with itself or the wall.
  bool collision = block_snake_collision(candidatePosition, game->snake);
  if (collision || !in_bounds(candidatePosition, game_dim)) {
    game_end(game);
    return;
  }

  // Check if the snake has collided with the apple.
  if (block_snake_collision(game->apple, game->snake)) {
    snake_add_head(game->snake, game->direction);
    game->position = snake_head(game->snake);
    game_reset_apple(game);
    game->score += 1;
  }
  // If the snake has not, move normally.
  else {
    snake_move(game->snake, game->direction);
    game->position = snake_head(game->snake);
  }
}
