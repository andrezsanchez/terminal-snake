#include "server.h"

#undef ns
#define ns(x) FLATBUFFERS_WRAP_NAMESPACE(Snake_Message, x)

// A helper to simplify creating vectors from C-arrays.
#define c_vec_len(V) (sizeof(V)/sizeof((V)[0]))

struct conn_t {
  int fd;
};

struct timespec t;
static void wait_please() {
  t.tv_sec = 0;
  t.tv_nsec = 50000000L;
  /*t.tv_nsec = 100000000L;*/
  nanosleep(&t, NULL);
}

uint8_t direction = 0;

// global to signal each thread to end
int ending = 0;

void * connection_handler(void * conn_pointer) {
  struct conn_t * conn = (struct conn_t *) conn_pointer;
  int sock = conn->fd;

  uint8_t buf[1] = {0};
  uint8_t * cur = buf;

  /*int rlen = read(sock, buf, sizeof(buf));*/
  /*checkio(rlen > -1, "read error");*/

  /*while (true) {*/
  /*printf("Handling connection\n");*/

  while (!ending) {
    int read_len = read(sock, buf, sizeof(buf));
    checkio(read_len > -1, "read error");
    /*printf("Received %d bytes\n", read_len);*/


    direction = buf[0] % 4;
    printf("set d=%d\n", direction);
  }

  /*uint16_t * len;*/
  /*// TODO: Make sure we read enough bytes to even do this.*/
  /*len = (uint16_t *) buf;*/

  /*printf("Next message should be %d bytes\n", *len);*/

  /*printf("Exiting thread\n");*/

  /*pthread_exit(NULL);*/
  return NULL;

error:
  if (sock > -1) close(sock);
  pthread_exit(NULL);
  return NULL;
}

vec2i get_direction(int ch) {
  int x = (ch == 1) - (ch == 3);
  int y = (ch == 2) - (ch == 0);
  return (vec2i) { x, y };
}

static void finish(int sig) {
  ending = 1;
}

int main() {
  signal(SIGTERM, finish);

  const int port = 8080;

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

  struct sockaddr_in client;
  int sl = sizeof(struct sockaddr_in);

  game_t game = {0};
  game_init(&game);

  int client_sock = accept(serversock, (struct sockaddr *) &client, (socklen_t *) &sl);
  checkio(client_sock > -1, "accept error");

  pthread_t t_id;

  struct conn_t * arg = malloc(sizeof(struct conn_t));
  checkio(arg != NULL, "malloc failure");

  arg->fd = client_sock;

  check(
    pthread_create(&t_id, NULL, connection_handler, (void *) arg) == 0,
    "pthread_create error"
  );

  int t = 0;
  while (!ending) {
    uint8_t dir = direction;
    if (!game.end_screen) {
      game_apply_direction(&game, get_direction(dir));
    }

    t += 1;

    write(client_sock, &dir, 1);

    // Prevent the game from going too fast.
    wait_please();
  }

  if (serversock > -1) {
    close(serversock);
  }
  serversock = -1;

  check(pthread_join(t_id, NULL) > -1, "Failed to join");

  pthread_exit(NULL);
  return 0;
error:
  if (serversock > -1) close(serversock);
  serversock = -1;
  pthread_exit(NULL);
  return 1;
}
