#!/usr/bin/env python3
"""
进程地址空间探测器
解析 /proc/PID/maps 并反汇编可执行区域
"""

import sys
import re
from pathlib import Path
from dataclasses import dataclass
from typing import Optional

# 可选的反汇编引擎 capstone
try:
    from capstone import *
    HAS_CAPSTONE = True
except ImportError:
    HAS_CAPSTONE = False


@dataclass
class MapRegion:
    """描述一个内存映射区域"""
    start: int          # 起始地址
    end: int            # 结束地址
    perms: str          # 权限位 rwxsp-
    offset: int         # 文件内偏移
    dev: str            # 设备号
    inode: int          # inode 编号
    pathname: Optional[str]  # 映射的文件路径（匿名映射为 None）

    @property
    def size(self) -> int:
        return self.end - self.start

    @property
    def is_executable(self) -> bool:
        return 'x' in self.perms

    @property
    def is_readable(self) -> bool:
        return 'r' in self.perms


def parse_maps(pid: int) -> list[MapRegion]:
    """解析 /proc/PID/maps，返回 MapRegion 列表"""
    maps_path = Path(f"/proc/{pid}/maps")
    if not maps_path.exists():
        raise FileNotFoundError(f"进程不存在: {pid}")

    regions = []
    pattern = re.compile(
        r'^([0-9a-f]+)-([0-9a-f]+)\s+([rwxps-]{4})\s+([0-9a-f]+)\s+(\S+)\s+(\d+)\s*(.*)$'
    )

    for line in maps_path.read_text().strip().split('\n'):
        match = pattern.match(line)
        if not match:
            continue

        start, end, perms, offset, dev, inode, pathname = match.groups()
        pathname = pathname.strip() or None

        regions.append(MapRegion(
            start=int(start, 16),
            end=int(end, 16),
            perms=perms,
            offset=int(offset, 16),
            dev=dev,
            inode=int(inode),
            pathname=pathname
        ))

    return regions


def describe_region(region: MapRegion) -> str:
    """返回内存区域的人类可读描述"""
    if region.pathname:
        # 栈
        if region.pathname == '[stack]':
            return "主线程栈"
        if region.pathname.startswith('[stack:'):
            return "子线程栈"
        # 堆
        if region.pathname == '[heap]':
            return "堆"
        # 匿名映射（内核 6.x+ 标注了用途）
        if region.pathname.startswith('[anon:'):
            return f"匿名映射 ({region.pathname[6:-1]})"
        # 内核特殊映射：vdso、vvar、vsyscall 等
        if region.pathname.startswith('[') and region.pathname.endswith(']'):
            return region.pathname[1:-1].capitalize()
        # 共享库
        if '.so' in Path(region.pathname).name:
            return f"共享库: {region.pathname}"
        # 普通文件映射
        return f"文件映射: {region.pathname}"
    return "匿名内存"


def get_arch(pid: int) -> tuple[int, int]:
    """探测目标进程的 CPU 架构，返回 (arch, mode) 供 capstone 使用"""
    exe_path = Path(f"/proc/{pid}/exe")

    # 默认 x86-64
    arch, mode = CS_ARCH_X86, CS_MODE_64

    if not exe_path.exists():
        return arch, mode

    try:
        # 读取 ELF 文件头
        header = exe_path.read_bytes()[:20]
        if len(header) < 20 or header[:4] != b'\x7fELF':
            return arch, mode

        ei_class = header[4]   # 1=32位, 2=64位
        ei_data = header[5]    # 1=小端, 2=大端
        e_machine = int.from_bytes(header[18:20], 'little' if ei_data == 1 else 'big')

        if e_machine == 0x3e:   # x86-64
            arch, mode = CS_ARCH_X86, CS_MODE_64
            if ei_class == 1:
                mode = CS_MODE_32
        elif e_machine == 0x28:  # ARM (32位)
            arch, mode = CS_ARCH_ARM, CS_MODE_LITTLE_ENDIAN
        elif e_machine == 0xb7:  # AArch64
            arch, mode = CS_ARCH_ARM64, CS_MODE_LITTLE_ENDIAN
        elif e_machine == 0xf3:  # RISC-V
            arch = CS_ARCH_RISCV
            mode = CS_MODE_RISCV64 if ei_class == 2 else CS_MODE_RISCV32
    except Exception:
        pass

    return arch, mode


def disassemble_region(pid: int, region: MapRegion, max_bytes: int = 1024) -> None:
    """反汇编可执行区域的前 max_bytes 字节"""
    if not HAS_CAPSTONE:
        print("    [需要安装 capstone: pip install capstone]")
        return

    mem_path = Path(f"/proc/{pid}/mem")
    if not mem_path.exists():
        print("    [无法访问 /proc/PID/mem]")
        return

    try:
        with open(mem_path, 'rb') as f:
            f.seek(region.start)
            code = f.read(min(max_bytes, region.size))
    except (PermissionError, OSError) as e:
        print(f"    [无法读取内存: {e}]")
        return

    if not code:
        print("    [区域为空]")
        return

    arch, mode = get_arch(pid)

    try:
        md = Cs(arch, mode)
        md.detail = False
        md.skipdata = True  # 遇到未知指令时跳过而不是停止

        print(f"    前 {len(code)} 字节反汇编结果:")
        for insn in md.disasm(code, region.start):
            bytes_str = ' '.join(f'{b:02x}' for b in insn.bytes[:8])
            if len(insn.bytes) > 8:
                bytes_str += '...'
            print(f"      0x{insn.address:016x}: {bytes_str:<20} {insn.mnemonic} {insn.op_str}")
    except Exception as e:
        print(f"    [反汇编错误: {e}]")


def main():
    if len(sys.argv) != 2:
        print(f"用法: {sys.argv[0]} <进程PID>")
        sys.exit(1)

    try:
        pid = int(sys.argv[1])
    except ValueError:
        print(f"无效 PID: {sys.argv[1]}")
        sys.exit(1)

    try:
        regions = parse_maps(pid)
    except FileNotFoundError as e:
        print(f"错误: {e}")
        sys.exit(1)
    except PermissionError:
        print(f"权限不足: 无法读取 /proc/{pid}/maps")
        print("系统进程请用 sudo 运行")
        sys.exit(1)

    print(f"进程 {pid} 地址空间（共 {len(regions)} 个区域）")
    print("=" * 80)

    for i, r in enumerate(regions, 1):
        print(f"\n[{i}] 0x{r.start:016x}-0x{r.end:016x}（{r.size:,} 字节）")
        print(f"    权限: {r.perms}")
        print(f"    描述: {describe_region(r)}")

        if r.pathname and r.offset > 0:
            print(f"    文件偏移: 0x{r.offset:x}")

        if r.is_executable and r.is_readable:
            disassemble_region(pid, r)


if __name__ == "__main__":
    main()
