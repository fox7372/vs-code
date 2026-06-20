CC = gcc
CFLAGS = -Wall -pthread

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  # Compile tests with flat namespace so DYLD_INSERT_LIBRARIES overrides symbols
  LDFLAGS = -Wl,-flat_namespace
  PRELOAD = DYLD_INSERT_LIBRARIES=./lockdep.so
else
  LDFLAGS =
  PRELOAD = LD_PRELOAD=./lockdep.so
endif

all: lockdep.so locktrace.so philosopher pass_test

lockdep.so: lockdep.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $<

locktrace.so: locktrace.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $<

philosopher: philosopher.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

pass_test: pass_test.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

test: all
	$(PRELOAD) ./philosopher
	$(PRELOAD) ./pass_test

clean:
	rm -f lockdep.so locktrace.so philosopher pass_test

.PHONY: all clean test
