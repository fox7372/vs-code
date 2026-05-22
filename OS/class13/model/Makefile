CFLAGS := -O2 -g -I"../thread..."
LDFLAGS := -lpthread

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=)

all: $(OBJS)

%: %.c
	gcc $(CFLAGS) $< $(LDFLAGS) -o $@

clean:
	rm -f $(OBJS)
