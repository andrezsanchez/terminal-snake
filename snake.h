#include "deps/list/list.h"
#include "vec.h"

void snake_add_head(list_t *snake, vec2i direction);
void snake_remove_tail(list_t *snake);
void snake_move(list_t *snake, vec2i direction);
vec2i snake_head(list_t * snake);
bool block_snake_collision(vec2i position, list_t *snake);

#define GAME_SIZE 30
