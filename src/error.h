#include <string.h> // strerror_r

#define check(C, S, ...) \
  if (!(C)) { \
    fprintf(stderr, "[ERROR] %s:%d " S "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    goto error; \
  }

#define checkio(C, S, ...) \
  if (!(C)) { \
    int eno = errno; \
    char buf[256] = {0}; \
    strerror_r(eno, buf, sizeof(buf) / sizeof(char)); \
    fprintf(stderr, \
        "[ERROR] %s:%d (errno=%d)\n" \
        "  " S ":\n" \
        "  %s\n", \
        __FILE__, __LINE__, eno, \
        ##__VA_ARGS__, buf); \
    errno = 0; \
    goto error; \
  }
