#include <stdlib.h>
#include <stdio.h>
#include <curses.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include "list/list.h"
#include "vec.h"
#include "game.h"
#include "snake.h"

static void finish(int sig);
static void screen_size_changed();

const int KEY_ESC = 27;

const int SNAKE_COLOR = 1;
const int APPLE_COLOR = 2;
const int WALL_COLOR = 3;

vec2i screen_offset(const vec2i screen_size) {
  return (vec2i) {
    (screen_size.x / 2) - (GAME_SIZE),
    (screen_size.y / 2) - (GAME_SIZE / 2),
  };
}

void draw_block(vec2i * block, const vec2i offset, const int color) {
  attron(COLOR_PAIR(color));
  mvwaddch(stdscr, block->y + offset.y, offset.x + block->x * 2, ' ');
  mvwaddch(stdscr, block->y + offset.y, offset.x + (block->x * 2) + 1, ' ');
  attroff(COLOR_PAIR(color));
}

void draw_snake(list_t * snake, const vec2i screen_size) {
  if (!snake) {
    return;
  }
  list_iterator_t * it = {0};
  
  if (!(it = list_iterator_new(snake, LIST_HEAD))) {
    return;
  }

  list_node_t * node = {0};
  while ((node = list_iterator_next(it))) {
    draw_block(node->val, screen_offset(screen_size), SNAKE_COLOR);
  }

  list_iterator_destroy(it);
}

struct timespec t;
static void wait_please() {
  t.tv_sec = 0;
  t.tv_nsec = 50000000L;
  nanosleep(&t, NULL);
}

bool is_right(int c) {
  switch (c) {
    case 'l':
    case 'd':
    case KEY_RIGHT:
      return true;
    default:
      return false;
  }
}

bool is_up(int c) {
  switch (c) {
    case 'k':
    case 'w':
    case KEY_UP:
      return true;
    default:
      return false;
  }
}

bool is_down(int c) {
  switch (c) {
    case 'j':
    case 's':
    case KEY_DOWN:
      return true;
    default:
      return false;
  }
}

bool is_left(int c) {
  switch (c) {
    case 'h':
    case 'a':
    case KEY_LEFT:
      return true;
    default:
      return false;
  }
}

vec2i get_direction(int ch) {
  if (ch != ERR) {
    if (ch == KEY_ESC) finish(0);
    int x = is_right(ch) - is_left(ch);
    int y = is_down(ch) - is_up(ch);
    return (vec2i) { x, y };
  }
  else {
    return (vec2i) { 0, 0 };
  }
}

void fill_background(const vec2i screen_size) {
  const vec2i min = screen_offset(screen_size);
  vec2i max = {0};
  vec2i_add(&max, min, (vec2i) { GAME_SIZE * 2, GAME_SIZE });

  attron(COLOR_PAIR(WALL_COLOR));
  for (int y = 0; y < screen_size.y; ++y) {
    for (int x = 0; x < screen_size.x; ++x) {
      if ((x < min.x) || (y < min.y) || (x >= max.x) || (y >= max.y)) {
        mvwaddch(stdscr, y, x, ' ');
      }
    }
  }
  attroff(COLOR_PAIR(WALL_COLOR));
}

#define SCORE_MESSAGE_SIZE 30
void draw_score(const game_t game) {
  char message[SCORE_MESSAGE_SIZE] = {0};
  snprintf(message, SCORE_MESSAGE_SIZE, "Score: %d", game.score);
  attron(COLOR_PAIR(WALL_COLOR));
  mvwaddstr(stdscr, 0, 0, message);
  attroff(COLOR_PAIR(WALL_COLOR));
}

const char YOU_LOSE_STRING[] = "You have failed your prerogative as a snake. Shame be upon you and your children.";

void draw_screen(game_t game, vec2i screen_size) {
  // Clear the screen.
  erase();

  // Write the you lose string if we're on the end screen.
  if (game.end_screen) {
    const int x = (screen_size.x / 2) - (sizeof(YOU_LOSE_STRING) / 2);
    const int y = screen_size.y / 2;
    mvwaddstr(stdscr, y, x, YOU_LOSE_STRING);
  }
  // Otherwise draw the game.
  else {
    // Fill the out-of-bounds part of the screen.
    fill_background(screen_size);

    draw_score(game);

    // Draw the snake.
    draw_snake(game.snake, screen_size);

    // Draw the apple.
    draw_block(&game.apple, screen_offset(screen_size), APPLE_COLOR);
  }

  // Copy the new contents to the screen.
  refresh();
}

void init_ncurses() {
  initscr();
  keypad(stdscr, TRUE);
  nonl();
  cbreak();

  // Do not "echo" what the user types onto the screen.
  noecho();

  // Initialize ncurses colors.
  start_color();
  init_pair(WALL_COLOR, COLOR_BLACK, COLOR_WHITE);
  init_pair(APPLE_COLOR, COLOR_RED, COLOR_RED);
  init_pair(SNAKE_COLOR, COLOR_YELLOW, COLOR_YELLOW);
}

void update_screen_size(vec2i * screen_size) {
  getmaxyx(stdscr, screen_size->y, screen_size->x);
}

int main(int argc, char **argv) {
  init_ncurses();

  signal(SIGINT, finish);
  signal(SIGWINCH, screen_size_changed);

  vec2i screen_size = {0};

  // Get the screen size.
  update_screen_size(&screen_size);

  // Hide the cursor.
  curs_set(0); 

  nodelay(stdscr, true);
  game_t game = {0};
  game_init(&game);

  for (;;) {
    // Prevent the game from going too fast.
    wait_please();

    // If we're on the end screen.
    if (game.end_screen) {
      int ch = getch();
      if (ch != ERR) {
        if (ch == 'q') {
          finish(0);
        }
        else {
          game_init(&game);
        }
      }
    }
    // Otherwise apply the direction and let the game code take care of what to do.
    else {
      const vec2i direction = get_direction(getch());
      game_apply_direction(&game, direction);
    }

    update_screen_size(&screen_size);
    draw_screen(game, screen_size);
  }

  finish(0);
}

static void finish(int sig) {
  endwin();
  exit(0);
}

static void screen_size_changed() {
  // Destroy and reinitialize the ncurses context so that it uses the new screen size.
  endwin();
  init_ncurses();
}
