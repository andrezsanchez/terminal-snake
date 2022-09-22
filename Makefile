CC = clang
FLATCC = ~/code/etc/flatcc/bin/flatcc

LIBS = c ncurses pthread # flatccrt

LDFLAGS = $(LIBS:%=-l%) # -Llib/flatcc
CFLAGS = -std=c18 -Wall -Iinclude -g

INCLUDE = $(wildcard include/*/*.c)
INCLUDE_OBJS = $(INCLUDE:.c=.o)
SRC = $(wildcard src/*.c) $(wildcard include/*/*.c)

DEFINES = _POSIX_C_SOURCE=200809L
DFLAGS = $(DEFINES:%=-D%)

OBJS = $(SRC:.c=.o)

BINS = main server

FB_INCLUDES = include/fb/flatbuffers_common_builder.h include/fb/flatbuffers_common_reader.h
FB = src/flatbuffers/snake.fbs

all: $(BINS)

#$(BINS): $(OBJS)
	#$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

GAME_OBJS = src/game.o src/snake.o src/vec.o
main: src/main.o $(GAME_OBJS) $(INCLUDE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(DFLAGS)

server: src/server.o $(GAME_OBJS) $(INCLUDE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(DFLAGS)
#server: 

%.o: %.c
	$(CC) $< -c -o $@ $(CFLAGS) $(DFLAGS)

#%_builder.h: %.fbs

#fb: $(FB_INCLUDES)
fb:
	$(FLATCC) $(FB) -a -o $(dir $(FB))

#$(FB_INCLUDES):
	#$(FLATCC) -c -o include/fb/

clean:
	rm -f $(BINS) $(OBJS) $(FB_INCLUDES)

.PHONY: all clean
