CFLAGS := -O1 -g -I$(TLIB_PATH)

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=)

# Default target
all: $(OBJS)

%: %.c
	gcc $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJS)

run: all
	@echo ----- Shared Memory Test -----
	@./test_shm | sort
	@echo ----- Stack Probe Test -----
	@./test_stack | sort -nk 3  | tail -n 1
