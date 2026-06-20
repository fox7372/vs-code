CFLAGS  := -O2
CXXFLAGS := -O2 -std=c++20

SRCS  := $(wildcard *.c)
CXXS  := $(wildcard *.cpp)
OBJS  := $(SRCS:.c=) $(CXXS:.cpp=)

# Default target
all: $(OBJS)

%: %.c
	gcc $(CFLAGS) $< -o $@

%: %.cpp
	g++ $(CXXFLAGS) $< -o $@

clean:
	rm -f $(OBJS)

