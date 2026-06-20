CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS =

TARGET = map
OBJS   = map.o hash.o
INPUT  = map.txt

.PHONY: all clean rebuild run test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

map.o: map.c map.h hash.h queue.h stack.h

hash.o: hash.c hash.h

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	./$(TARGET) < $(INPUT)

rebuild: clean all
