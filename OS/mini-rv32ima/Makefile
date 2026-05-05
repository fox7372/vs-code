# mini-rv32ima: RISC-V32IMA 模拟器构建文件

mini-rv32ima: main.c mini-rv32ima.h
		gcc -g -o $@ $<

# RV32I 汇编 → 原始二进制
hello.rv32i-bin: hello.S
		riscv64-linux-gnu-as -march=rv32i -mabi=ilp32 -o hello.o $<
		riscv64-linux-gnu-ld -m elf32lriscv -Ttext=0 -o hello.elf hello.o
		riscv64-linux-gnu-objcopy -O binary hello.elf $@
		rm -f hello.o hello.elf

run-hello: mini-rv32ima hello.rv32i-bin
		./mini-rv32ima hello.rv32i-bin

clean:
		rm -f mini-rv32ima hello.rv32i-bin hello.o hello.elf

.PHONY: clean run-hello
