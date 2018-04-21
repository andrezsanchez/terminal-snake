#include <stdlib.h>
#include <curses.h>
#include <signal.h>
#include "list/list.h"

#include <time.h>
#include <unistd.h>

static void finish(int sig);
static void screenSizeChange();
static void handleInput();

typedef struct {
  int x;
  int y;
} vector2;

vector2 * vector2_new(int x, int y) {
  vector2 * self = malloc(sizeof(vector2));
  self->x = x;
  self->y = y;
  return self;
}

const int KEY_ESC = 27;

void cap(int *x, int *y, int w, int h) {
  if (*x < 0) *x = 0;
  if (*x >= w) *x = w - 1;
  if (*y < 0) *y = 0;
  if (*y >= h) *y = h - 1;
}

bool inBounds(int x, int y, int w, int h) {
  if (x < 0) return false;
  if (x >= w) return false;
  if (y < 0) return false;
  if (y >= h) return false;
  return true;
}

void drawBlock(vector2 *block, int color) {
  attron(COLOR_PAIR(color));
  mvwaddch(stdscr, block->y, block->x, ' ');
  mvwaddch(stdscr, block->y, block->x+1, ' ');
  attroff(COLOR_PAIR(color));
}

void drawSnake(list_t *snake) {
  list_node_t *node;
  list_iterator_t *it = list_iterator_new(snake, LIST_HEAD);

  while ((node = list_iterator_next(it))) {
    drawBlock(node->val, 1);
  }
  list_iterator_destroy(it);
}

bool blockSnakeCollision(int x, int y, list_t *snake) {
  list_node_t *node;
  list_iterator_t *it = list_iterator_new(snake, LIST_HEAD);

  bool collision = false;

  vector2 * block;
  while ((node = list_iterator_next(it))) {
    block = (vector2 *) node->val;
    if (block->x == x && block->y == y) {
      collision = true;
      break;
    }
  }
  list_iterator_destroy(it);

  return collision;
}

bool moveSnake(list_t *snake, vector2 position) {
  list_node_t * n = list_rpop(snake);

  bool collision = blockSnakeCollision(position.x, position.y, snake);

  list_lpush(snake, n);
  ((vector2 *) snake->head->val)->x = position.x;
  ((vector2 *) snake->head->val)->y = position.y;

  return collision;
}

int screenw;
int screenh;

/*list_t *snake = NULL;*/

struct timespec t;
static void waitPlease() {
  t.tv_sec = 0;
  t.tv_nsec = 50000000L;
  nanosleep(&t, NULL);
}

typedef struct {
  list_t * snake;
  vector2 position;
  vector2 direction;
  bool endScreen;
  vector2 apple;
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
  for (int i = 0; i < 30; i += 1) {
    list_rpush(
      game->snake,
      list_node_new(
        vector2_new(game->position.x+i*2, game->position.y)
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

void gameApplyDirection(Game *game, vector2 direction) {
  if (
      !(game->direction.x == direction.x && game->direction.y == -direction.y) &&
      !(game->direction.x == -direction.x && game->direction.y == direction.y) &&
      !(direction.x == 0 && direction.y == 0)
     ) {
    game->direction = direction;
  }
  game->position.x += game->direction.x*2;
  game->position.y += game->direction.y;
}

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

  getmaxyx(stdscr, screenh, screenw);

  //hide cursor
  curs_set(0); 

  nodelay(stdscr, true);

  vector2 screen = (vector2) { .x = screenw, .y = screenh };
  Game game;
  gameInit(&game);

  vector2 newDirection;
  for (;;) {
    waitPlease();
    if (!game.endScreen) {
      handleInput(&newDirection);
      gameApplyDirection(&game, newDirection);

      bool collision = moveSnake(game.snake, game.position);
      if (collision || !inBounds(game.position.x, game.position.y, screenw, screenh)) {
        game.endScreen = true;
        gameEnd(&game);
        continue;
      }

      /*cap(&x, &y, screenw, screenh);*/

      erase();
      drawSnake(game.snake);
      drawBlock(&game.apple, 2);
      /*mvwaddch(stdscr, 20, 20, '@');*/
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
      const int x = screenw / 2;
      const int y = screenh / 2;
      const char youLose[] = "You have failed you're prerogative as a snake. Shame be upon you and your children.";
      mvwaddstr(stdscr, y, x - (sizeof(youLose) / 2), youLose);
    }
    refresh();
  }

  finish(0);
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

static void handleInput(vector2 *output) {
  int ch = getch();
  if (ch != ERR) {
    if (ch == KEY_ESC) finish(0);
    int x = right(ch) - left(ch);
    int y = isDown(ch) - isUp(ch);
    output->x = x;
    output->y = y;
  }
  else {
    output->x = 0;
    output->y = 0;
  }
}

static void finish(int sig) {
  /*gameEnd();*/
  endwin();
  exit(0);
}

static void screenSizeChange() {
}
