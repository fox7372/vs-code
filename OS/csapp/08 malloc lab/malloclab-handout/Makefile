#
# Makefile for CS:APP Malloc Lab
#
# Targets:
#   make           — 编译 mm.c（单线程原始版）
#   make fixed     — 编译 mm_fixed.c（线程安全版）
#   make test      — 运行所有 trace
#

TEAM = liu
VERSION = 1

CC = gcc
CFLAGS = -Wall -O2 -g
LDFLAGS = -lpthread

SRC = src
TEST = traces
TRACEDIR = traces/

TRACES = short1-bal.rep short2-bal.rep

# 默认使用 mm.c，make fixed 切换为 mm_fixed.c
MM_SRC = mm.c
MM_OBJ = $(SRC)/mm.o

OBJS = $(TEST)/mdriver.o $(MM_OBJ) $(SRC)/memlib.o $(TEST)/fsecs.o $(TEST)/fcyc.o $(TEST)/clock.o $(TEST)/ftimer.o

# ============ 目标 ============

.PHONY: all build fixed test test-all clean handin valgrind debug help

all: build

build: mdriver

fixed:
	$(MAKE) build MM_SRC=mm_fixed.c MM_OBJ=$(SRC)/mm_fixed.o

SEG_OBJS = $(TEST)/mdriver.o $(SRC)/mm_segregated.o $(SRC)/memlib.o $(TEST)/fsecs.o $(TEST)/fcyc.o $(TEST)/clock.o $(TEST)/ftimer.o

seg: CFLAGS += -Wl,--allow-multiple-definition
seg: $(SEG_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o mdriver-seg $(SEG_OBJS) -lpthread

$(SRC)/mm_segregated.o: $(SRC)/mm_segregated.c $(SRC)/mm.h $(SRC)/memlib.h
	$(CC) $(CFLAGS) -DDRIVER -I$(SRC) -c $< -o $@

test-seg: seg
	@for trace in $(TRACES); do \
		echo ">>> Running $$trace..."; \
		./mdriver-seg -V -f $(TRACEDIR)$$trace; \
		echo; \
	done

NSEG_OBJS = $(TEST)/mdriver.o $(SRC)/mm_segregated_nosplit.o $(SRC)/memlib.o $(TEST)/fsecs.o $(TEST)/fcyc.o $(TEST)/clock.o $(TEST)/ftimer.o

nosplit: CFLAGS += -Wl,--allow-multiple-definition
nosplit: $(NSEG_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o mdriver-nosplit $(NSEG_OBJS) -lpthread

$(SRC)/mm_segregated_nosplit.o: $(SRC)/mm_segregated_nosplit.c $(SRC)/mm.h $(SRC)/memlib.h
	$(CC) $(CFLAGS) -DDRIVER -I$(SRC) -c $< -o $@

test-nosplit: nosplit
	@for trace in $(TRACES); do \
		echo ">>> Running $$trace..."; \
		./mdriver-nosplit -V -f $(TRACEDIR)$$trace; \
		echo; \
	done

# 多线程测试（mm_segregated_nosplit.c 版）
mt-nosplit: CFLAGS += -Wl,--allow-multiple-definition
mt-nosplit:
	$(CC) -Wall -g -O0 -Isrc -I$(HOME)/.local/include \
		-o mt_test_nosplit src/mm_segregated_nosplit.c src/memlib.c crash_demo.c \
		-lpthread -Wl,--allow-multiple-definition
	@echo ""
	@echo "=== nosplit 多线程测试 ==="
	@./mt_test_nosplit; echo "exit code: $$?"

# 多线程测试（debug.c 版）
mt-debug: CFLAGS += -Wl,--allow-multiple-definition
mt-debug:
	$(CC) -Wall -g -O0 -Isrc -I$(HOME)/.local/include \
		-o mt_test_debug src/debug.c src/memlib.c crash_demo.c \
		-lpthread -Wl,--allow-multiple-definition
	@echo ""
	@echo "=== debug 多线程测试 ==="
	@./mt_test_debug; echo "exit code: $$?"

DBG_OBJS = $(TEST)/mdriver.o $(SRC)/debug.o $(SRC)/memlib.o $(TEST)/fsecs.o $(TEST)/fcyc.o $(TEST)/clock.o $(TEST)/ftimer.o

update-debug: CFLAGS += -Wl,--allow-multiple-definition
update-debug: $(DBG_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o mdriver-debug $(DBG_OBJS) -lpthread

$(SRC)/debug.o: $(SRC)/debug.c $(SRC)/mm.h $(SRC)/memlib.h
	$(CC) $(CFLAGS) -DDRIVER -I$(SRC) -c $< -o $@

test-debug: update-debug
	@for trace in $(TRACES); do \
		echo ">>> Running $$trace..."; \
		./mdriver-debug -V -f $(TRACEDIR)$$trace; \
		echo; \
	done

# 多线程测试（mm_segregated.c 版）
mt-seg: seg
	$(CC) -Wall -g -O0 -Isrc -I$(HOME)/.local/include \
		-o mt_test_seg src/mm_segregated.c src/memlib.c crash_demo.c \
		-lpthread -Wl,--allow-multiple-definition
	@echo ""
	@echo "=== 多线程测试 ==="
	@./mt_test_seg; echo "exit code: $$?"

mdriver: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) -lpthread

$(TEST)/mdriver.o: $(TEST)/mdriver.c $(TEST)/fsecs.h $(TEST)/fcyc.h $(TEST)/clock.h $(SRC)/memlib.h $(SRC)/config.h $(SRC)/mm.h
	$(CC) $(CFLAGS) -I$(SRC) -I$(TEST) -c $< -o $@

$(SRC)/mm.o: $(SRC)/mm.c $(SRC)/mm.h $(SRC)/memlib.h
	$(CC) $(CFLAGS) -DDRIVER -I$(SRC) -c $< -o $@

$(SRC)/mm_fixed.o: $(SRC)/mm_fixed.c $(SRC)/mm.h $(SRC)/memlib.h
	$(CC) $(CFLAGS) -DDRIVER -I$(SRC) -c $< -o $@

$(SRC)/memlib.o: $(SRC)/memlib.c $(SRC)/memlib.h
	$(CC) $(CFLAGS) -I$(SRC) -c $< -o $@

$(TEST)/fsecs.o: $(TEST)/fsecs.c $(TEST)/fsecs.h $(SRC)/config.h
	$(CC) $(CFLAGS) -I$(SRC) -I$(TEST) -c $< -o $@

$(TEST)/fcyc.o: $(TEST)/fcyc.c $(TEST)/fcyc.h
	$(CC) $(CFLAGS) -I$(TEST) -c $< -o $@

$(TEST)/ftimer.o: $(TEST)/ftimer.c $(TEST)/ftimer.h $(SRC)/config.h
	$(CC) $(CFLAGS) -I$(SRC) -I$(TEST) -c $< -o $@

$(TEST)/clock.o: $(TEST)/clock.c $(TEST)/clock.h
	$(CC) $(CFLAGS) -I$(TEST) -c $< -o $@

# 测试
test: mdriver
	@for trace in $(TRACES); do \
		echo ">>> Running $$trace..."; \
		./mdriver -V -f $(TRACEDIR)$$trace; \
		echo; \
	done

test-all: mdriver
	./mdriver -t $(TRACEDIR) -a

%.test: mdriver
	./mdriver -V -f $(TRACEDIR)$*

%.test-quiet: mdriver
	./mdriver -f $(TRACEDIR)$*

debug: CFLAGS = -Wall -O0 -g
debug: clean build
	./mdriver -V -f $(TRACEDIR)short1-bal.rep

valgrind: build
	valgrind --leak-check=full --show-leak-kinds=all \
		./mdriver -V -f $(TRACEDIR)short1-bal.rep

handin:
	@echo "Creating handin file: $(TEAM)-$(VERSION)-mm.c"
	cp $(SRC)/mm.c $(TEAM)-$(VERSION)-mm.c

# 多线程崩溃演示（使用 thread.h 的 spawn/join）
# mm.c 和 memlib.c 都包含 thread-sync.h（无头文件保护），
# 加 --allow-multiple-definition 让链接器忽略重复的 spin_lock/spin_unlock
crash: crash_demo.c $(SRC)/mm.c $(SRC)/memlib.c
	$(CC) -Wall -O2 -g -Isrc -I$(HOME)/.local/include -o $@ \
		crash_demo.c $(SRC)/mm.c $(SRC)/memlib.c \
		-lpthread -Wl,--allow-multiple-definition
	@echo ""
	@echo "=== 多线程 crash 演示 ==="
	@./crash; echo "exit code: $$?"

clean:
	rm -f *~ *.o $(SRC)/*.o $(TEST)/*.o mdriver

help:
	@echo "Usage: make [target]"
	@echo ""
	@echo "  make           编译 mm.c"
	@echo "  make fixed     编译 mm_fixed.c（线程安全版）"
	@echo "  make seg       编译 mm_segregated.c（分离链表，有分裂）"
	@echo "  make nosplit   编译 mm_segregated_nosplit.c（无分裂）"
	@echo "  make test      运行所有 trace"
	@echo "  make test-seg  运行 trace（mm_segregated.c 版）"
	@echo "  make test-nosplit 运行 trace（nosplit 版）"
	@echo "  make mt-seg    多线程测试（mm_segregated.c 版）"
	@echo "  make mt-nosplit 多线程测试（nosplit 版）"
	@echo "  make update-debug 编译 debug.c → mdriver-debug"
	@echo "  make test-debug   运行 trace（debug.c 版）"
	@echo "  make mt-debug     多线程测试（debug.c 版）"
	@echo "  make debug     调试模式"
	@echo "  make valgrind  内存检测"
	@echo "  make clean     清理"
