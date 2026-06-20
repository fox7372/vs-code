.PHONY: run replay trace record clean load

# 一键启动 TUI（自动启动 bpftrace → bio-state 管线）
run:
	./bio-tui

# 回放录制的追踪日志
replay:
	./bio-tui --replay trace.log

# 仅采集 bpftrace 原始数据（调试用）
trace:
	sudo bpftrace bio-trace.bt

# 录制追踪数据到文件（供 replay 使用）
record:
	sudo bpftrace bio-trace.bt > trace.log

# 生成 I/O 测试负载（在另一终端运行）
load:
	dd if=/dev/zero of=/tmp/bio-load.tmp bs=4096 count=500 conv=fsync 2>/dev/null && sync && rm -f /tmp/bio-load.tmp

clean:
	rm -f /tmp/bio-load*.tmp trace.log
