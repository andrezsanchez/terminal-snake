#include <stdbool.h>
#include <stdio.h>
#include "list/list.h"
#include "vec.h"
#include "snake.h"

//enum Something { A, B };

/**
 * The game object. All of the game state is encapsulated in this object.
 */
typedef struct {
  list_t * snake;
  vec2i position;
  vec2i direction;
  bool end_screen;
  vec2i apple;
  int score;
} game_t;

void game_init(game_t * game);
void game_end(game_t * game);
void game_destroy(game_t * game);
void game_set_apple(game_t * game, const vec2i position);
void game_apply_direction(
  game_t * game,
  const vec2i direction,
  const int random_value
);

void game_print(game_t * game, FILE * stream);
