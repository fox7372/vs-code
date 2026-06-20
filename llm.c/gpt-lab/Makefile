NAME := $(shell basename $(PWD))
export MODULE := M3
all: $(NAME)-64 $(NAME)-32
LDFLAGS := -lm

### add debugging information
CFLAGS += -g

include ../Makefile
