#!/usr/bin/env python3
"""检查 eat.c 的并发问题：死锁、数据竞争、筷子冲突"""

import subprocess
import sys
import os
import re
import time
from collections import defaultdict

EXE = os.path.join(os.path.dirname(__file__), "eat")
SRC = os.path.join(os.path.dirname(__file__), "eat.c")
N = 5
RUNS = 20
TIMEOUT = 5  # 秒，超时即判定为死锁


def compile():
    print("[*] 编译 eat.c ...")
    ret = subprocess.run(
        ["gcc", "-o", EXE, SRC, "-lpthread", "-O2", "-g", "-fsanitize=thread"],
        capture_output=True, text=True
    )
    if ret.returncode != 0:
        print("[-] 编译失败，尝试无 sanitizer 编译")
        ret = subprocess.run(
            ["gcc", "-o", EXE, SRC, "-lpthread", "-O2"],
            capture_output=True, text=True
        )
        if ret.returncode != 0:
            print(ret.stderr)
            sys.exit(1)
    print("[+] 编译成功\n")


def parse_output(text: str) -> list[dict]:
    """解析每一行输出为结构化事件"""
    events = []
    for line in text.strip().split("\n"):
        line = line.strip()
        if not line:
            continue
        m = re.match(r"\[(\w+)\]\s*(.*)", line)
        if not m:
            continue
        tag = m.group(1)
        rest = m.group(2)
        if tag == "Think":
            m2 = re.search(r"Philosopher (\d+)", rest)
            if m2:
                events.append({"type": "think", "id": int(m2.group(1))})
        elif tag == "Wait":
            m2 = re.search(r"Philosopher (\d+)", rest)
            if m2:
                events.append({"type": "wait", "id": int(m2.group(1))})
        elif tag == "Eat":
            m2 = re.search(r"Philosopher (\d+).*chopsticks (\d+) & (\d+)", rest)
            if m2:
                events.append({
                    "type": "eat", "id": int(m2.group(1)),
                    "left": int(m2.group(2)), "right": int(m2.group(3))
                })
        elif tag == "Done":
            m2 = re.search(r"Philosopher (\d+)", rest)
            if m2:
                events.append({"type": "done", "id": int(m2.group(1))})
    return events


def check_concurrent_eating(events: list[dict]) -> list[str]:
    """检查是否有两个相邻哲学家同时持有一根筷子"""
    errors = []
    eat_intervals = {}  # id -> (start_index, chopsticks)

    for idx, ev in enumerate(events):
        if ev["type"] == "eat":
            eat_intervals[ev["id"]] = (idx, ev["left"], ev["right"])
        elif ev["type"] == "done":
            if ev["id"] in eat_intervals:
                # 检查在 (start, idx) 区间内有无冲突
                start, l, r = eat_intervals.pop(ev["id"])
                for j in range(start + 1, idx):
                    e2 = events[j]
                    if e2["type"] == "eat":
                        # 检查筷子冲突
                        if l in (e2["left"], e2["right"]) or r in (e2["left"], e2["right"]):
                            errors.append(
                                f"  筷子冲突: P{ev['id']} 用 ({l},{r}) 期间 "
                                f"P{e2['id']} 也在用 ({e2['left']},{e2['right']})"
                            )
    # 未闭合的区间
    for pid, (start, l, r) in eat_intervals.items():
        for j in range(start + 1, len(events)):
            e2 = events[j]
            if e2["type"] == "eat":
                if l in (e2["left"], e2["right"]) or r in (e2["left"], e2["right"]):
                    errors.append(
                        f"  筷子冲突: P{pid} 用 ({l},{r}) 期间 "
                        f"P{e2['id']} 也在用 ({e2['left']},{e2['right']})"
                    )
    return errors


def check_cf_outside_lock(text: str) -> list[str]:
    """
    静态分析: 检查 cf[i] 赋值是否在锁外.
    用简单的行特征检测.
    """
    notes = []
    lines = text.split("\n")
    in_unlock_region = False
    for i, line in enumerate(lines):
        stripped = line.strip()
        if "pthread_mutex_unlock" in stripped:
            in_unlock_region = True
        if in_unlock_region and re.match(r"cf\[", stripped):
            notes.append(f"  第 {i+1} 行: '{stripped}' — cf 在 mutex_unlock 之后修改，存在数据竞争")
        if "pthread_mutex_lock" in stripped:
            in_unlock_region = False
        if "pthread_cond_broadcast" in stripped and "mutex" not in stripped:
            notes.append(f"  第 {i+1} 行: '{stripped}' — cond_broadcast 在锁外调用")
    return notes


def detect_deadlock(log: str) -> bool:
    """检查日志中是否有所有哲学家都在等待的死锁模式"""
    wait_count = len(re.findall(r"waiting for a dining", log, re.I))
    eat_count = len(re.findall(r"is eating", log, re.I))
    done_count = len(re.findall(r"finished eating", log, re.I))
    return wait_count > 0 and eat_count == 0 and done_count == 0


def run_test() -> dict:
    """单次运行 eat，返回结果"""
    start = time.perf_counter()
    try:
        proc = subprocess.run(
            [EXE], capture_output=True, text=True,
            timeout=TIMEOUT
        )
        elapsed = time.perf_counter() - start
        return {
            "ok": True,
            "timeout": False,
            "elapsed": elapsed,
            "stdout": proc.stdout,
            "deadlock": detect_deadlock(proc.stdout),
        }
    except subprocess.TimeoutExpired:
        elapsed = time.perf_counter() - start
        return {
            "ok": True,
            "timeout": True,
            "elapsed": elapsed,
            "stdout": "",
            "deadlock": True,
        }


def main():
    compile()

    results = []
    print(f"[*] 运行 {RUNS} 次，每次超时 {TIMEOUT}s ...\n")

    for i in range(RUNS):
        r = run_test()
        results.append(r)
        status = "超时(死锁)" if r["deadlock"] else f"完成({r['elapsed']:.2f}s)"
        print(f"  运行 {i+1:2d}/{RUNS}: {status}")

    # 汇总
    timeouts = sum(1 for r in results if r["timeout"])
    finishes = RUNS - timeouts
    print(f"\n{'='*50}")
    print(f"[结果]")
    print(f"  总运行: {RUNS} 次")
    print(f"  正常完成: {finishes} 次")
    print(f"  超时(死锁): {timeouts} 次")

    if timeouts > 0:
        print(f"\n[!] 检测到死锁! 所有哲学家同时拿左边筷子时互相等待。")
        print(f"    这是经典 Dining Philosophers 死锁问题。")

    # 分析正常完成的输出
    if finishes > 0:
        print(f"\n{'='*50}")
        print("[运行时分析]")
        all_errors = set()
        for r in results:
            if r["timeout"]:
                continue
            events = parse_output(r["stdout"])
            errors = check_concurrent_eating(events)
            all_errors.update(errors)

        # 静态分析
        with open(SRC) as f:
            src_text = f.read()
        static_notes = check_cf_outside_lock(src_text)

        print(f"\n[静态分析 — 代码问题]")
        with open(SRC) as f:
            src_lines = f.readlines()

        print(f"\n  关键代码段 (哲学家吃饭/放筷子逻辑):")
        for i, line in enumerate(src_lines[17:33], start=18):
            print(f"  {i:2d}: {line}", end="")

        print(f"\n  发现的问题:")
        if static_notes:
            for n in static_notes:
                print(f"  [!] {n}")
        else:
            print(f"  [√] 未发现明显静态问题")

        print(f"\n[动态分析 — 运行时冲突]")
        if all_errors:
            print(f"  [!] 发现 {len(all_errors)} 次筷子冲突:")
            for e in sorted(all_errors):
                print(f"  {e}")
        else:
            print(f"  [√] 未检测到运行时筷子冲突")

    print(f"\n{'='*50}")
    print("[结论]")
    if timeouts > 0:
        print("  该程序存在死锁问题 (+ 数据竞争)，建议:")
        print("    - 限制同时就餐人数（如加一个信号量 N-1）")
        print("    - 或让哲学家先拿编号小的筷子（资源有序分配）")
        print("    - 将 cf 赋值和 cond_broadcast 移到 mutex 内")
    else:
        print("  该次测试未触发死锁，但静态分析仍需关注。")


if __name__ == "__main__":
    main()
