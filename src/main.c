#include "main.h"

/*#undef ns*/
/*#define ns(x) FLATBUFFERS_WRAP_NAMESPACE(Snake_Message, x)*/

// A helper to simplify creating vectors from C-arrays.
#define c_vec_len(V) (sizeof(V)/sizeof((V)[0]))

static void finish(int sig);
static void screen_size_changed();

const int KEY_ESC = 27;

const int SNAKE_COLOR = 1;
const int APPLE_COLOR = 2;
const int WALL_COLOR = 3;

struct listener_context_t {
  int fd;
  uint8_t command;
};

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

uint8_t get_direction_uint8(vec2i dir_v) {
  if (dir_v.y == -1) {
    return 0;
  }

  if (dir_v.y == 1) {
    return 2;
  }

  if (dir_v.x == 1) {
    return 1;
  }

  if (dir_v.x == -1) {
    return 3;
  }

  return 5;
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

void draw_text_center(
  const vec2i position,
  const char * text,
  size_t text_len
) {
  mvwaddstr(stdscr, position.y, position.x - (text_len / 2), text);
}

#define SCORE_MESSAGE_SIZE 15
void draw_score(const game_t game, const vec2i position) {
  char message[SCORE_MESSAGE_SIZE] = {0};
  snprintf(message, SCORE_MESSAGE_SIZE, "Score: %d", game.score);

  size_t len = strnlen(message, SCORE_MESSAGE_SIZE);

  draw_text_center(position, message, len);
  /*mvwaddstr(stdscr, 1, 1, message);*/
}

const char YOU_LOSE_STRING[] = "You have failed your prerogative as a snake. Shame be upon you and your children.";

void draw_screen(game_t game, vec2i screen_size) {
  // Clear the screen.
  erase();

  // Fill the out-of-bounds part of the screen.
  fill_background(screen_size);

  // Draw the snake.
  draw_snake(game.snake, screen_size);

  // Draw the apple.
  draw_block(&game.apple, screen_offset(screen_size), APPLE_COLOR);

  // Write the you lose string if we're on the end screen.
  if (game.end_screen) {
    attron(COLOR_PAIR(APPLE_COLOR));
    draw_text_center(
      (vec2i) {
        (screen_size.x / 2),
        screen_size.y / 2
      },
      YOU_LOSE_STRING,
      sizeof(YOU_LOSE_STRING)
    );
    draw_score(game, (vec2i) { (screen_size.x / 2), (screen_size.y / 2) + 1 });
    attroff(COLOR_PAIR(APPLE_COLOR));
  }
  // Otherwise draw the game.
  else {
    attron(COLOR_PAIR(WALL_COLOR));
    draw_score(game, (vec2i) { screen_size.x / 2, 1 });
    attroff(COLOR_PAIR(WALL_COLOR));
  }

  // Copy the new contents to the screen.
  refresh();
}

void init_ncurses() {
  initscr();

  // Allows arrow keys to be used as input.
  keypad(stdscr, TRUE);

  nonl();

  // The cbreak routine disables line buffering and erase/kill character-processing (interrupt and
  // flow control characters are unaffected), making characters typed by the user immediately
  // available to the program
  cbreak();

  // Do not "echo" what the user types onto the screen.
  noecho();

  // Initialize ncurses colors.
  start_color();

  // Causes `getch` to be a non-blocking call. If no input is ready `getch` will return `ERR`.
  nodelay(stdscr, true);

  // Hide the cursor.
  curs_set(0);

  init_pair(WALL_COLOR, COLOR_BLACK, COLOR_WHITE);
  init_pair(APPLE_COLOR, COLOR_BLACK, COLOR_RED);
  init_pair(SNAKE_COLOR, COLOR_YELLOW, COLOR_YELLOW);
}

void update_screen_size(vec2i * screen_size) {
  getmaxyx(stdscr, screen_size->y, screen_size->x);
}

int conn_open(const char * host, const char * port) {
  struct addrinfo * addr = NULL;
  int sock = -1;

  {
    int result = getaddrinfo(host, port, NULL, &addr);
    checkio(result == 0, "getaddrinfo error: %s", gai_strerror(result));
  }

  sock = socket(AF_INET, SOCK_STREAM, 0);
  checkio(sock >= 0, "could not open socket");

  int rv = connect(sock, addr->ai_addr, addr->ai_addrlen);
  checkio(rv == 0, "could not connect");

  freeaddrinfo(addr);
  return sock;
error:
  if (addr) freeaddrinfo(addr);
  if (sock > 0) close(sock);
  return -1;
}

int ending = 0;

vec2i direction_from_byte(uint8_t ch) {
  int x = (ch == 1) - (ch == 3);
  int y = (ch == 2) - (ch == 0);
  return (vec2i) { x, y };
}

void * listener(void * conn_pointer) {
  struct listener_context_t * listener_context = (struct listener_context_t *) conn_pointer;

  uint8_t buf[1024] = {0};
  while (!ending) {

    int read_len = read(listener_context->fd, buf, sizeof(buf));
    checkio(read_len > -1, "read error");

    if (read_len == 0) {
      continue;
    }

    if (read_len > 1) {
      ending = true;
      break;
    }

    listener_context->command = buf[0];
  }

  return NULL;
error:
  ending = true;
  return NULL;
}

int main(int argc, char **argv) {
  init_ncurses();

  signal(SIGINT, finish);
  signal(SIGWINCH, screen_size_changed);

  vec2i screen_size = {0};

  // Get the screen size.
  update_screen_size(&screen_size);

  game_t game = {0};
  game_init(&game);

  // Connect to the server.
  const char host[] = "127.0.0.1";
  const char port[] = "8080";
  int sock = conn_open(host, port);
  check(sock > -1, "Failed to connect");

  struct listener_context_t * listener_context = malloc(sizeof(struct listener_context_t));
  checkio(listener_context != NULL, "malloc failure");
  listener_context->command = 254;
  listener_context->fd = sock;

  pthread_t listener_thread;
  check(
    pthread_create(&listener_thread, NULL, listener, (void *) listener_context) == 0,
    "Could not create thread"
  );

  srand(0);

  bool dirty = true;
  while (!ending) {
    uint8_t command = listener_context->command;
    listener_context->command = 253;
    if (command != 253) {
      if (command == 255) {
        game_init(&game);
      } else {
        vec2i direction = direction_from_byte(command);
        int random_value = rand() % (GAME_SIZE * GAME_SIZE);
        game_apply_direction(&game, direction, random_value);
      }
      dirty = true;
    }

    const int input = getch();
    if (
      (input != ERR && input == 'q') ||
      (game.end_screen && input == 'q')
    ) {
      const uint8_t reset = 255;
      checkio(
        write(sock, &reset, 1) > -1,
        "Could not write"
      );
    } else if (!game.end_screen) {
      const vec2i direction = get_direction(input);
      const uint8_t dir = get_direction_uint8(direction);
      if (dir != 5) {
        checkio(
          write(sock, &dir, 1) > -1,
          "Could not write"
        );
      }
    }

    if (dirty) {
      update_screen_size(&screen_size);
      draw_screen(game, screen_size);
      dirty = false;
    }
  }

  pthread_join(listener_thread, NULL);

  if (sock > -1) close(sock);
  sock = -1;
  finish(0);

  return 0;
error:
  if (sock > -1) close(sock);
  finish(0);
  sock = -1;

  return 1;
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
