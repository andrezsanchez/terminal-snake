#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h> // NULL
#include <stdio.h> // printf
#include <sys/socket.h> // socket
#include <unistd.h> // close
#include <signal.h>
#include <stdbool.h>
#include <sched.h>

#include "error.h"
#include "stream.h"

#include "game.h"
