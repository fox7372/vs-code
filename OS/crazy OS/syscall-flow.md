p1.c / p2.c 系统调用流程
==========================

文件关系
  p1.c / p2.c   — RISC-V 程序，运行在模拟的 RISC-V CPU 上
  lib.h         — RISC-V 程序库，封装 ecall 指令
  mini-rv32ima.h — RISC-V CPU 模拟器核心，逐条解释 RISC-V 指令
  crazy-os.c    — 加载器 + 系统调用处理器（运行在 x86 上）

调用链
  p1.c → lib.h (myprintf → sys_putchar → ecall)
       → mini-rv32ima.h (捕获 ecall，设 MCAUSE=8)
       → crazy-os.c (handle_ecall → 输出字符到终端)

详细流程

  Step 1: p1.c 调用 myprintf
      myprintf("P1: x = %d\n", x);

  Step 2: lib.h 解析格式字符串，逐字符调用 sys_putchar
      sys_putchar 把字符放到 a0，服务号 42 放到 a7，执行 ecall
        a0 = 字符的 ASCII 值
        a7 = 42（系统调用号，表示 putchar）

  Step 3: mini-rv32ima.h 的 rv32ima_step() 执行 ecall 指令
      触发异常: MCAUSE = 8（来自 U 模式的 ECALL）
      MEPC = 当前 PC（ecall 指令的地址）

  Step 4: crazy-os.c 主循环检测到 MCAUSE == 8(这是一个结构体指针的内容lib.h通过mini-rv32调用函数修改，这个修改会传递到crazy os)
      调用 handle_ecall()
      根据 a7 = 42 分发到 sys_putchar()
      把字符写入 stdout → 显示在终端屏幕上
      恢复执行: PC = MEPC + 4（ecall 的下一条指令）

示意图

  p1.c (RISC-V)    lib.h (RISC-V)    mini-rv32ima.h     crazy-os.c (x86)
      |                 |                 |                   |
      | myprintf(...)   |                 |                   |
      | ─────────────→  |                 |                   |
      |                 | sys_putchar(c)  |                   |
      |                 | a0=ch, a7=42    |                   |
      |                 | ecall ────────→ |                   |
      |                 |                 | MCAUSE=8          |
      |                 |                 | ────────────────→ |
      |                 |                 |                   | handle_ecall()
      |                 |                 |                   | a7==42 → fputchar
      |                 |                 |                   | → 输出到终端
      |                 |                 |                   | PC = MEPC+4
      |                 |                 | ←──── 返回 ────── |
      |                 | ←── 继续执行 ── |                   |
      |   ←── 恢复 ──── |                 |                   |

关键点
  1. p1/p2 自己不输出，只发 ecall 指令
  2. crazy-os.c 运行在 x86 上，真正负责输出到终端
  3. 参数通过寄存器传递: a0 = 字符, a7 = 服务号
  4. 调度方式: 轮转（round-robin），p1 和 p2 各执行一条指令后切换
