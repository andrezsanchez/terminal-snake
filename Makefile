CC = colorgcc

LDFLAGS = -lncurses
CFLAGS = -std=gnu99 -Wall -Ideps

DEPS = $(wildcard deps/*/*.c)
OBJS = $(DEPS:.c=.o)

SRC = main.c

BINS = main

all: $(BINS)

$(BINS): $(SRC) $(OBJS)
	$(CC) $(CFLAGS) $@.c -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $< -c -o $@ $(CFLAGS)

clean:
	rm $(BINS) $(OBJS)

.PHONY: all clean
