#include <stdlib.h>
#include <curses.h>
#include <signal.h>
#include "list/list.h"

#include <time.h>
#include <unistd.h>

static void finish(int sig);
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

void drawBlock(vector2 *block) {
  attron(COLOR_PAIR(1));
  mvwaddch(stdscr, block->y, block->x, ' ');
  mvwaddch(stdscr, block->y, block->x+1, ' ');
  attroff(COLOR_PAIR(1));
}

void drawSnake(list_t *snake) {
  list_node_t *node;
  list_iterator_t *it = list_iterator_new(snake, LIST_HEAD);

  while ((node = list_iterator_next(it))) {
    drawBlock(node->val);
  }
  list_iterator_destroy(it);
}

void moveSnake(list_t *snake, int x, int y) {
  list_node_t * n = list_rpop(snake);
  list_lpush(snake, n);
  ((vector2 *) snake->head->val)->x = x;
  ((vector2 *) snake->head->val)->y = y;
}

int screenw;
int screenh;

list_t *snake;

struct timespec t;
static void wait() {
  t.tv_sec = 0;
  t.tv_nsec = 50000000L;
  nanosleep(&t, NULL);
}

int directionX = 0;
int directionY = 1;

int main(int argc, char **argv) {
  int x = 3;
  int y = 3;

  snake = list_new();
  snake->free = free;

  /*list_node_t *head =*/
  for (int i = 0; i < 60; i+=1) {
    list_rpush(snake, list_node_new(vector2_new(x+i*2,y)));
  }

  signal(SIGINT, finish);

  initscr();
  keypad(stdscr, TRUE);
  nonl();
  cbreak();
  noecho();
  start_color();
  init_pair(1, COLOR_RED, COLOR_WHITE);

  getmaxyx(stdscr, screenh, screenw);

  //hide cursor
  curs_set(0); 

  nodelay(stdscr, true);

  for (;;) {
    wait();
    handleInput();
    x += directionX*2;
    y += directionY;
    cap(&x, &y, screenw, screenh);

    moveSnake(snake, x, y);
    erase();
    drawSnake(snake);
    refresh();
  }

  finish(0);
}

static void handleInput() {
  int ch = getch();
  if (ch != ERR) {
    if (ch == KEY_ESC) finish(0);
    int up = (ch == KEY_RIGHT) - (ch == KEY_LEFT);
    int down = (ch == KEY_DOWN) - (ch == KEY_UP);
    if (up || down) {
      directionX = up;
      directionY = down;
    }
  }
}

static void finish(int sig) {
  list_destroy(snake);
  endwin();
  exit(0);
}
