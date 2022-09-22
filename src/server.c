#include "server.h"

// A helper to simplify creating vectors from C-arrays.
#define c_vec_len(V) (sizeof(V)/sizeof((V)[0]))

struct conn_t {
  int fd;
  uint8_t * direction;
};

struct game_context_t {
  int fd;
  uint8_t * direction;
  bool ending;
};

struct timespec t;
static void wait_please() {
  t.tv_sec = 0;
  t.tv_nsec = 50000000L;
  nanosleep(&t, NULL);
}

// uint8_t direction = 0;

// global to signal each thread to end
int ending = 0;

vec2i get_direction(int ch) {
  int x = (ch == 1) - (ch == 3);
  int y = (ch == 2) - (ch == 0);
  return (vec2i) { x, y };
}

void * game_handler(void * game_context_ptr) {
  struct game_context_t * game_context = (struct game_context_t *) game_context_ptr;
  int sock = game_context->fd;

  printf("Game handler\n");

  game_t game = {0};
  game_init(&game);
  game_print(&game, stdout);

  srand(0);

  while (!game_context->ending) {
    u_int8_t direction = *game_context->direction;

    switch (direction) {
      case 255: {
        *game_context->direction = 254;
        printf("restart\n");

        game_init(&game);
        break;
      }
      default: {
        vec2i direction_v = {0,0};
        if (direction != 254) {
          printf("d(%u)\n", direction);
          direction_v = get_direction(direction % 4);
        }

        *game_context->direction = 254;

        game_apply_direction(&game, direction_v, rand());
        break;
      }
    }

    write(sock, &direction, 1);

    wait_please();
  }

  printf("Game handler close\n");

  return NULL;
}

void * connection_handler(void * conn_pointer) {
  struct conn_t * conn = (struct conn_t *) conn_pointer;
  int sock = conn->fd;

  uint8_t buf[1] = {0};
  /*uint8_t * cur = buf;*/

  /*int rlen = read(sock, buf, sizeof(buf));*/
  /*checkio(rlen > -1, "read error");*/

  /*while (true) {*/
  printf("Connection handler\n");

  while (!ending) {
    int read_len = read(sock, buf, sizeof(buf));
    checkio(read_len != -1, "read error");

    // 0 means the connection is closed
    if (read_len == 0) {
      break;
    }

    if (read_len > 1) {
      printf("read_len > 1 wtf");
      ending = true;
    }

    for (int i = 0; i < read_len; i += 1) {
      // uint8_t direction = buf[0];
      *conn->direction = buf[i];
    }
  }

  /*uint16_t * len;*/
  /*// TODO: Make sure we read enough bytes to even do this.*/
  /*len = (uint16_t *) buf;*/

  /*printf("Next message should be %d bytes\n", *len);*/

  printf("Connection handler close\n");
  return NULL;

error:
  printf("Connection handler close\n");
  if (sock > -1) close(sock);
  pthread_exit(NULL);
  return NULL;
}

static void finish(int sig) {
  ending = 1;
}

int main() {
  signal(SIGTERM, finish);

  const uint16_t port = 8080;

  int serversock = socket(AF_INET, SOCK_STREAM, 0);
  checkio(serversock > -1, "could not open socket");

  struct sockaddr_in server = {
    .sin_family = AF_INET,
    .sin_addr = {
      .s_addr = INADDR_ANY,
    },
    .sin_port = htons(port),
  };

  checkio(
    bind(serversock, (struct sockaddr *) &server, sizeof(server)) > -1,
    "bind error"
  );

  checkio(listen(serversock, 3) > -1, "listen error");

  while (!ending) {
    struct sockaddr_in client;
    int sl = sizeof(struct sockaddr_in);

    int client_sock = accept(serversock, (struct sockaddr *) &client, (socklen_t *) &sl);
    checkio(client_sock > -1, "accept error");

    uint8_t * direction = malloc(sizeof(uint8_t));
    *direction = 0;

    // Create connection handler thread.

    pthread_t connection_handler_thread;
    struct conn_t * connection_context = malloc(sizeof(struct conn_t));
    checkio(connection_context != NULL, "malloc failure");

    connection_context->fd = client_sock;
    connection_context->direction = direction;
    check(
      pthread_create(&connection_handler_thread, NULL, connection_handler, (void *) connection_context) == 0,
      "pthread_create error"
    );

    // Create game handler thread.

    pthread_t game_handler_thread;
    struct game_context_t * game_context = malloc(sizeof(struct game_context_t));
    checkio(game_context != NULL, "malloc failure");
    game_context->fd = client_sock;
    game_context->direction = direction;
    check(
      pthread_create(&game_handler_thread, NULL, game_handler, (void *) game_context) == 0,
      "pthread_create error"
    );

    // Join threads.

    check(pthread_join(connection_handler_thread, NULL) > -1, "Failed to join");
    game_context->ending = true;

    check(pthread_join(game_handler_thread, NULL) > -1, "Failed to join");
  }

  if (serversock > -1) {
    close(serversock);
  }
  serversock = -1;

  pthread_exit(NULL);
  return 0;
error:
  if (serversock > -1) close(serversock);
  serversock = -1;
  pthread_exit(NULL);
  return 1;
}
