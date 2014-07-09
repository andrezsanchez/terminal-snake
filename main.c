#include <stdlib.h>
#include <curses.h>
#include <signal.h>
#include "list/list.h"

#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */
#include <time.h>
#include <unistd.h>

static void finish(int sig);

struct Point {
  int x;
  int y;
};

struct SnakeCell {
  struct Point pos;
  struct SnakeCell *next;
};

struct Snake {
  struct Point snake[20];
  int start;
  int end;
};

const int KEY_ESC = 27;

void cap(int *x, int *y, int w, int h) {
  if (*x < 0) *x = 0;
  if (*x >= w) *x = w - 1;
  if (*y < 0) *y = 0;
  if (*y >= h) *y = h - 1;
}

void drawBlock(struct Point block) {
  mvwaddch(stdscr, block.y, block.x, ACS_BLOCK);
  mvwaddch(stdscr, block.y, block.x+1, ACS_BLOCK);
}
void drawSnake(struct Point snake[], int ln) {
  for (int i=0; i < ln; i+=1) {
    drawBlock(snake[i]);
  }
}
void moveSnake(struct Point snake[], int ln, struct Point to) {

}

int
main(int argc, char *argv[]) {
  int i;
  int x = 0;
  int y = 0;

  struct Point snake[20];
  /*struct Snake sn;*/
  /*for (i=0; i<20; i+=1) {*/
    /*sn.snake[i].x = i;*/
    /*sn.snake[i].y = 0;*/
  /*}*/
  /*sn.start = 0;*/
  /*sn.end = 1;*/

  //int snakeln = 9;
  for (i=0; i<20; i+=1) {
    snake[i].x = i;
    snake[i].y = 0;
  }


  int directionX = 1;
  int directionY = 0;

  (void) signal(SIGINT, finish);
  (void) initscr();
  keypad(stdscr, TRUE);
  (void) nonl();
  (void) cbreak();
  (void) noecho();
  start_color();
  
  int screenw;
  int screenh;
  getmaxyx(stdscr, screenh, screenw);

  //hide cursor
  curs_set(0); 

  nodelay(stdscr, true);

  mvwaddch(stdscr, y, x, 'X');

  struct timespec t;
  for (;;) {
    t.tv_sec = 0;
    t.tv_nsec = 50000000L;
    nanosleep(&t, NULL);
    int ch = getch();
    if (ch != ERR) {
      if (ch == KEY_ESC) break;
      directionX = (ch == KEY_RIGHT) - (ch == KEY_LEFT);
      directionY = (ch == KEY_DOWN) - (ch == KEY_UP);
    }
    x += directionX*2;
    y += directionY;
    cap(&x, &y, screenw, screenh);

    erase();
    mvwaddch(stdscr, y, x, ACS_BLOCK);
    mvwaddch(stdscr, y, x+1, ACS_BLOCK);
    //drawSnake(sn, snakeln);
    refresh();
  }

  finish(0);
}

static void finish(int sig) {
  endwin();
  exit(0);
}
