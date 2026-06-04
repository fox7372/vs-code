# Makefile — 编译管弦乐队的两个版本
# orchestra: 自旋等待版（忙等待）
# orchestra-cv: 条件变量版（高效）

CFLAGS := -O2 -I$(TLIB_PATH)

all: orchestra orchestra-cv

orchestra: orchestra.c main.c notes/C1.wav
	gcc $(CFLAGS) -o $@ $<

orchestra-cv: orchestra-cv.c main.c notes/C1.wav
	gcc $(CFLAGS) -o $@ $<

notes/C1.wav:            # 用 Python 脚本生成音符的 wav 音频文件
	python3 gen-notes.py

clean:
	rm -rf notes orchestra orchestra-cv
