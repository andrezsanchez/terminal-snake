#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h> // close
#include <stdlib.h>
#include <stdio.h>
#include <curses.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#include "error.h"
#include "list/list.h"
#include "vec.h"
#include "game.h"
#include "snake.h"

#include <sys/types.h>
#include <sys/socket.h> // socket
#include <netdb.h>

//#include "flatcc/flatcc_builder.h"
//#include "flatbuffers/snake_builder.h"
