CC = clang

LDFLAGS = -lncurses
CFLAGS = -std=c18 -Wall -Iinclude -g

SRC = $(wildcard src/*.c) $(wildcard include/*/*.c)

OBJS = $(SRC:.c=.o)

BINS = main

all: $(BINS)

$(BINS): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $< -c -o $@ $(CFLAGS)

clean:
	rm -f $(BINS) $(OBJS)

.PHONY: all clean
