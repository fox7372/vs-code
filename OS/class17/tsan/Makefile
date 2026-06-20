CFLAGS := -g -I$(TLIB_PATH)

# --- Original TSAN demo ---
race: race.c Makefile
	gcc -O2 -fsanitize=thread $(CFLAGS) -o $@ $<

# --- QEMU memory trace plugin (Linux only) ---
#
# Requires: qemu-user-static, glib-2.0-dev
#
# Ubuntu/Debian: apt install qemu-user-static libglib2.0-dev
#
# Adjust QEMU_PLUGINDIR if qemu-plugin.h is elsewhere:
#   make QEMU_PLUGINDIR=/usr/include qemu-memtrace.so

QEMU_PLUGINDIR ?= /usr/include
GLIB_CFLAGS   := $(shell pkg-config --cflags glib-2.0 2>/dev/null)
GLIB_LDFLAGS  := $(shell pkg-config --libs   glib-2.0 2>/dev/null)

qemu-memtrace.so: qemu-memtrace.c
	$(CC) -shared -fPIC -O2 -Wall -Wextra \
		-I$(QEMU_PLUGINDIR) $(GLIB_CFLAGS) \
		-o $@ $< $(GLIB_LDFLAGS)

trace-race: trace-race.c
	$(CC) -O0 $(CFLAGS) -o $@ $< -lpthread

clean:
	rm -f race qemu-memtrace.so trace-race
