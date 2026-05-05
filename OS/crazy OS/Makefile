CC       = gcc
CFLAGS   = -O2

RV_CC    = riscv64-linux-gnu-gcc
RV_FLAGS = -march=rv32ima -ffreestanding -nostdlib -static -mabi=ilp32 \
           -Wl,-Trv32.lds

all: crazy-os p1.bin p2.bin

crazy-os: crazy-os.c mini-rv32ima.h
	$(CC) $(CFLAGS) -o $@ $<

%.elf: %.c lib.h
	$(RV_CC) $(RV_FLAGS) -o $@ $<

%.bin: %.elf
	riscv64-linux-gnu-objcopy -O binary $< $@

run: crazy-os p1.bin p2.bin
	./crazy-os p1.bin p2.bin

clean:
	rm -f crazy-os *.elf *.bin

.PHONY: clean run
//执行过程 p1.c->p1.o与lib.h连接->p1.elf通过.lds->p1.bin最后通过crazy os完成系统调用

