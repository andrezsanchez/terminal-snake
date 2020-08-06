#include "snake.h"

void snake_add_head(list_t * snake, vec2i direction) {
  if (!snake) {
    return;
  }

  vec2i * head = vec2i_clone((vec2i *) snake->head->val);
  vec2i_add(head, *head, direction);
  list_node_t * node = list_node_new(head);
  list_lpush(snake, node);
}

void snake_remove_tail(list_t * snake) {
  if (!snake) {
    return;
  }

  list_remove(snake, snake->tail);
}

void snake_move(list_t * snake, vec2i direction) {
  if (!snake) {
    return;
  }

  snake_add_head(snake, direction);
  snake_remove_tail(snake);
}

vec2i snake_head(list_t * snake) {
  return *((vec2i *) snake->head->val);
}

bool block_snake_collision(vec2i position, list_t * snake) {
  if (!snake) {
    return false;
  }

  list_node_t * node;
  list_iterator_t * it = list_iterator_new(snake, LIST_HEAD);

  bool collision = false;

  vec2i * block;
  while ((node = list_iterator_next(it))) {
    block = (vec2i *) node->val;
    if (block->x == position.x && block->y == position.y) {
      collision = true;
      break;
    }
  }
  list_iterator_destroy(it);

  return collision;
}

