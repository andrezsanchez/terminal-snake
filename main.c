#include <stdlib.h>
#include <curses.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include "list/list.h"
#include "vec.h"

static void finish(int sig);
static void screenSizeChange();

const int KEY_ESC = 27;

bool in_bounds(const vec2i position, const vec2i screen_size) {
  if (position.x < 0) return false;
  if (position.x >= screen_size.x) return false;
  if (position.y < 0) return false;
  if (position.y >= screen_size.y) return false;
  return true;
}

void draw_block(vec2i * block, int color) {
  attron(COLOR_PAIR(color));
  mvwaddch(stdscr, block->y, block->x * 2, ' ');
  mvwaddch(stdscr, block->y, (block->x * 2) + 1, ' ');
  attroff(COLOR_PAIR(color));
}

void draw_snake(list_t *snake) {
  list_iterator_t * it;
  
  if (!(it = list_iterator_new(snake, LIST_HEAD))) {
    return;
  }

  list_node_t * node;
  while ((node = list_iterator_next(it))) {
    draw_block(node->val, 1);
  }

  list_iterator_destroy(it);
}

bool block_snake_collision(vec2i position, list_t *snake) {
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

void snake_add_head(list_t *snake, vec2i direction) {
  vec2i * head = vec2i_clone((vec2i *) snake->head->val);
  vec2i_add(head, *head, direction);
  list_node_t * node = list_node_new(head);
  list_lpush(snake, node);
}

void snake_remove_tail(list_t *snake) {
  list_remove(snake, snake->tail);
}

void snakeMove(list_t *snake, vec2i direction) {
  snake_add_head(snake, direction);
  snake_remove_tail(snake);
}

vec2i snake_head(list_t * snake) {
  return *((vec2i *) snake->head->val);
}

struct timespec t;
static void waitPlease() {
  t.tv_sec = 0;
  t.tv_nsec = 50000000L;
  nanosleep(&t, NULL);
}

typedef struct {
  list_t * snake;
  vec2i position;
  vec2i direction;
  bool endScreen;
  vec2i apple;
} Game;

void gameInit(Game *game) {
  game->position.x = 10 + rand() % 25;
  game->position.y = 10 + rand() % 25;
  game->direction.x = 0;
  game->direction.y = 1;
  game->snake = list_new();
  game->snake->free = free;
  game->endScreen = false;
  game->apple.x = 10;
  game->apple.y = 10;
  for (int i = 0; i < 5; i += 1) {
    list_rpush(
      game->snake,
      list_node_new(
        vec2i_new(game->position.x + i, game->position.y)
      )
    );
  }
}

void gameEnd(Game *game) {
  if (game->snake != NULL) {
    list_destroy(game->snake);
  }
  game->snake = NULL;
}

void nextDirection(
  vec2i * currentDirection,
  const vec2i inputDirection
) {
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

void gameResetApple(Game *game) {
  do {
    game->apple.x = rand() % 30;
    game->apple.y = rand() % 30;
  } while (block_snake_collision(game->apple, game->snake));
}

bool right(int c) {
  switch (c) {
    case 'l':
    case 'd':
    case KEY_RIGHT:
      return true;
    default:
      return false;
  }
}

bool isUp(int c) {
  switch (c) {
    case 'k':
    case 'w':
    case KEY_UP:
      return true;
    default:
      return false;
  }
}

bool isDown(int c) {
  switch (c) {
    case 'j':
    case 's':
    case KEY_DOWN:
      return true;
    default:
      return false;
  }
}

bool left(int c) {
  switch (c) {
    case 'h':
    case 'a':
    case KEY_LEFT:
      return true;
    default:
      return false;
  }
}

vec2i getDirection(int ch) {
  if (ch != ERR) {
    if (ch == KEY_ESC) finish(0);
    int x = right(ch) - left(ch);
    int y = isDown(ch) - isUp(ch);
    return (vec2i) { x, y };
  }
  else {
    return (vec2i) { 0, 0 };
  }
}

const char youLose[] = "You have failed you're prerogative as a snake. Shame be upon you and your children.";

int main(int argc, char **argv) {
  signal(SIGINT, finish);
  signal(SIGWINCH, screenSizeChange);

  initscr();
  keypad(stdscr, TRUE);
  nonl();
  cbreak();
  noecho();
  start_color();
  init_pair(1, COLOR_RED, COLOR_WHITE);
  init_pair(2, COLOR_RED, COLOR_RED);

  vec2i screen;
  getmaxyx(stdscr, screen.y, screen.x);

  //hide cursor
  curs_set(0); 

  nodelay(stdscr, true);
  Game game;
  gameInit(&game);

  for (;;) {
    waitPlease();
    if (!game.endScreen) {
      nextDirection(
        &game.direction,
        getDirection(getch())
      );
      vec2i candidatePosition;
      vec2i_add(&candidatePosition, game.position, game.direction);

      bool collision = block_snake_collision(candidatePosition, game.snake);
      if (collision || !in_bounds(candidatePosition, screen)) {
        game.endScreen = true;
        gameEnd(&game);
        continue;
      }

      if (block_snake_collision(game.apple, game.snake)) {
        snake_add_head(game.snake, game.direction);
        game.position = snake_head(game.snake);
        gameResetApple(&game);
      }
      else {
        snakeMove(game.snake, game.direction);
        game.position = snake_head(game.snake);
      }

      erase();
      draw_snake(game.snake);
      draw_block(&game.apple, 2);
    }
    else {
      int ch = getch();
      if (ch != ERR) {
        if (ch == 'q') {
          finish(0);
        }
        else {
          gameEnd(&game);
          gameInit(&game);
        }
      }

      erase();
      const int x = screen.x / 2;
      const int y = screen.y / 2;
      mvwaddstr(stdscr, y, x - (sizeof(youLose) / 2), youLose);
    }
    refresh();
  }

  finish(0);
}

static void finish(int sig) {
  /*gameEnd();*/
  endwin();
  exit(0);
}

static void screenSizeChange() {
}
