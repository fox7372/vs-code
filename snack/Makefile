CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGETS = snake proc_dump

all: $(TARGETS)

snake: CFLAGS += $(shell pkg-config --cflags sdl2 2>/dev/null || echo "")
snake: LDFLAGS = $(shell pkg-config --libs sdl2 2>/dev/null || echo "-lSDL2")
snake: snake.c
	$(CC) $(CFLAGS) -o $@ snake.c $(LDFLAGS)

proc_dump: proc_dump.c
	$(CC) $(CFLAGS) -o $@ proc_dump.c

clean:
	rm -f $(TARGETS)

run: snake
	./snake

.PHONY: all clean run
