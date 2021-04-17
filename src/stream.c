#include "stream.h"
#include <stdio.h> // printf

int stream(int from_fd, int to_fd) {
  char buf[STREAM_BUFFER_SIZE] = {0};

  /*printf("stream start\n");*/

  while (1) {
    int len = read(from_fd, buf, STREAM_BUFFER_SIZE - 1);
    checkio(len > -1 && errno == 0, "read error");

    /*printf("streaming len=%d\n", len);*/
    if (len == 0) {
      /*printf("connection closed\n");*/
      break;
    }

    {
      int rv = write(to_fd, buf, len);
      checkio(rv > -1, "write error");
    }
  }

  /*printf("stream end\n");*/

  return 0;
error:
  return -1;
}
