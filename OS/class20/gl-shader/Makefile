CC = gcc
CFLAGS = -Wall -O2 -Iinclude $(shell pkg-config --cflags glfw3)
LDLIBS = $(shell pkg-config --libs glfw3) -ldl -lm

glshader: main.c include/gl.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f glshader
