# Mini libc — 使用裸机编译选项，不依赖任何 C 运行时库
CC      = gcc
CFLAGS  = -ffreestanding -nostdlib -static -fno-stack-protector \
          -O2 -g3 -Wall -Wextra
LDFLAGS = -ffreestanding -nostdlib -static

SRCS = stdio.c stdlib.c string.c
OBJS = crt0.o stdio.o stdlib.o string.o

.PHONY: all clean

# 默认目标：编译 demo 可执行文件
all: test

# ── 静态库（供其他程序链接：-lminicrt）──────────────────────────────
libc.a: stdio.o stdlib.o string.o
	ar rcs $@ $^

# ── demo 可执行文件 ─────────────────────────────────────────────────
test: test.o $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^
	@echo "+++ 构建完成: $@（未链接任何 libc）

# ── 编译规则 ─────────────────────────────────────────────────────────
# .c → .o：依赖 libc.h，修改头文件会自动重新编译
%.o: %.c libc.h
	$(CC) $(CFLAGS) -c -o $@ $<

# .S（汇编）→ .o
%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

# 清理所有编译产物
clean:
	rm -f *.o *.a test
