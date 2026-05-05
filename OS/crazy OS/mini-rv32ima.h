// Copyright 2022 Charles Lohr (MIT Licenced)
//
// 这是一个"嵌入式"RISC-V32IMA 系统模拟器。原始仓库：
// https://github.com/cnlohr/mini-rv32ima.
//
// 从《计算机系统概论》中得到的核心启示：
//
//     "一切都是状态机。"
//
// 参考：The RISC-V Reader (http://www.riscvbook.com/)

#include <stdint.h>

// ============================================================
// RISC-V 通用寄存器枚举
// ============================================================
// RISC-V RV32I 有 32 个通用寄存器 x0-x31，各有约定的用途。
enum RV32IMA_REG {
    Z,  // x0: 零寄存器。
    //     硬连线为 0，写入被忽略。
    RA, // x1: 返回地址寄存器。
    //     按约定存储函数调用的返回地址。jal 和 jalr 会在此保存返回地址。
    SP, // x2: 栈指针寄存器。
    //     指向内存中栈的顶部。
    GP, // x3: 全局指针寄存器。
    //     通常指向全局数据段的中间位置。
    TP, // x4: 线程指针寄存器。
    //     用于线程局部存储（TLS）。
    T0, // x5-x7: 临时寄存器。
    T1, //     汇编代码和编译器用于临时存储，跨函数调用无需保存。
    T2,
    S0, // x8-x9: 保存寄存器。
    S1, //     存储需要跨函数调用保留的值。
        //     在某些调用约定中，S0 也被用作帧指针（FP）。
    A0, // x10-x17: 参数寄存器。
    A1, //     用于传递函数参数（最多 8 个）。
    A2, //     A0 和 A1 也用于函数返回值。
    A3, A4, A5, A6, A7,
    S2, // x18-x27: 更多保存寄存器。
    S3, //     与 S0、S1 类似，存储需要跨函数调用保留的值。
    S4,
    S5, S6, S7, S8, S9, S10, S11,
    T3, // x28-x31: 更多临时寄存器。
    T4, //     与 T0-T2 类似，值不需要跨函数调用保留。
    T5,
    T6,
};

// ============================================================
// 控制和状态寄存器（CSR）枚举
// ============================================================
// CSR 是 RISC-V 中用于配置和控制处理器的特殊寄存器，
// 仅在机器模式（M-Mode）下可访问。
enum RV32IMA_CSR {
    PC,          // 程序计数器：当前指令的地址
    MSTATUS,     // 机器状态寄存器：全局中断使能、之前的特权模式及其他状态位
    CYCLEL,      // 周期计数器低 32 位：记录自复位以来经过的周期数
    CYCLEH,      // 周期计数器高 32 位
    TIMERL,      // 实时定时器低 32 位：以恒定速率递增
    TIMERH,      // 实时定时器高 32 位
    TIMERMATCHL, // 定时器匹配值低 32 位：用于触发定时器中断
    TIMERMATCHH, // 定时器匹配值高 32 位
    MSCRATCH,    // 机器模式暂存寄存器：供陷阱处理程序使用
    MTVEC,       // 机器模式陷阱向量基址寄存器：陷阱处理程序的入口地址
    MIE,         // 机器模式中断使能：控制哪些中断被启用
    MIP,         // 机器模式中断待决：指示哪些中断正在等待处理
    MEPC,        // 机器模式异常 PC：异常发生后要返回的地址
    MTVAL,       // 机器模式陷阱值：提供关于陷阱的附加信息
    MCAUSE,      // 机器模式陷阱原因：记录上一次陷阱的原因
    EXTRAFLAGS,  // 额外标志：处理器内部译码状态
                 // （非标准 RISC-V 规范定义）

    CSR_COUNT,   // CSR 总数：工具值，表示 CSR 的数量
};

// ============================================================
// CPU 状态结构体
// ============================================================
// 保存整个 RISC-V 处理器的状态，包括寄存器、CSR 和内存。
struct CPUState {
    // 处理器内部状态
    uint32_t regs[32], csrs[CSR_COUNT];

    // 内存状态
    uint8_t *mem;         // 指向模拟内存的指针
    uint32_t mem_offset;  // 内存起始偏移地址
    uint32_t mem_size;    // 内存大小
};

// ============================================================
// rv32ima_step — 单步执行一条 RISC-V 指令
// ============================================================
// 模拟 RISC-V 处理器执行一条指令的全部过程，包括：
//   - 定时器更新与中断检查
//   - 指令读取与译码
//   - 指令执行（支持 RV32I、RV32M、RV32A 扩展）
//   - 寄存器写回
//   - 陷阱和中断处理
//
// 参数:
//   state     — CPU 状态指针
//   elapsedUs — 自上次调用以来经过的微秒数（用于定时器）
//
// 返回:
//   0  — 正常执行
//   非零 — 程序退出码（通常由 SYSCON 设备写入）
static inline int32_t rv32ima_step(struct CPUState *state, uint32_t elapsedUs) {
    // 简化宏：方便访问 CSR、寄存器和内存
    #define CSR(x) (state->csrs[x])
    #define REG(x) (state->regs[x])
    #define MEM(x) (&state->mem[x])

    // ============================================================
    // 1. 定时器更新
    // ============================================================
    uint32_t new_timer = CSR(TIMERL) + elapsedUs;
    if (new_timer < CSR(TIMERL)) {
        CSR(TIMERH)++;  // 低 32 位溢出，高位加 1
    }
    CSR(TIMERL) = new_timer;

    // ============================================================
    // 2. 定时器中断检查
    // ============================================================
    uint64_t timer = ((uint64_t)CSR(TIMERH) << 32) | CSR(TIMERL);
    uint64_t timermatch = ((uint64_t)CSR(TIMERMATCHH) << 32) | CSR(TIMERMATCHL);

    if ((CSR(TIMERMATCHH) || CSR(TIMERMATCHL)) && (timer > timermatch)) {
        CSR(EXTRAFLAGS) &= ~4;      // 清除 WFI 标志
        CSR(MIP) |= 1 << 7;         // 设置机器定时器中断待决
    } else {
        CSR(MIP) &= ~(1 << 7);      // 清除机器定时器中断待决
    }

    // ============================================================
    // 3. WFI（等待中断）处理
    // ============================================================
    if (CSR(EXTRAFLAGS) & 4)
        return 1;  // 处理器处于休眠状态，直接返回

    uint32_t trap = 0;       // 陷阱类型（0 表示无陷阱）
    uint32_t rval = 0;       // 指令执行结果值
    uint32_t pc = CSR(PC);   // 当前程序计数器
    uint32_t cycle = CSR(CYCLEL);  // 当前周期数

    // ============================================================
    // 4. 定时器中断注入
    // ============================================================
    if ((CSR(MIP) & (1 << 7)) && (CSR(MIE) & (1 << 7) /* MTIE */) && (CSR(MSTATUS) & 0x8 /* MIE */)) {
        trap = 0x80000007;  // 机器定时器中断（最高位为 1 表示中断而非异常）
        pc -= 4;            // 回退 PC，以便在 cycle_end 中正确处理
        goto cycle_end;
    }

    // ============================================================
    // 5. 指令读取与译码执行
    // ============================================================
    uint32_t ir = 0;      // 指令编码
    rval = 0;
    cycle++;
    uint32_t ofs_pc = pc - state->mem_offset;

    // 检查指令地址是否有效
    if (ofs_pc >= state->mem_size) {
        trap = 1 + 1;  // 指令读取访问违例
        goto cycle_end;
    } else if (ofs_pc & 3) {
        trap = 1 + 0;  // PC 未对齐
        goto cycle_end;
    } else {
        ir = *(uint32_t *)MEM(ofs_pc);  // 读取指令
        uint32_t rdid = (ir >> 7) & 0x1f;  // 目标寄存器编号（rd）

        // ============================================================
        // 主译码器：根据 opcode (ir[6:0]) 分发
        // ============================================================
        switch (ir & 0x7f) {
        case 0x37: // LUI (opcode = 0b0110111)
            // 加载高位立即数：将 20 位立即数加载到寄存器的高 20 位
            rval = (ir & 0xfffff000); break;
        case 0x17: // AUIPC (opcode = 0b0010111)
            // PC 加高位立即数：将 PC 加上 20 位立即数
            rval = pc + (ir & 0xfffff000); break;
        case 0x6F: { // JAL (opcode = 0b1101111)
            // 跳转并链接：跳转到 PC + 偏移，返回地址存入 rd
            int32_t reladdy = ((ir & 0x80000000) >> 11) | ((ir & 0x7fe00000) >> 20) | ((ir & 0x00100000) >> 9) | ((ir & 0x000ff000));
            if (reladdy & 0x00100000)
                reladdy |= 0xffe00000; // 符号扩展
            rval = pc + 4;          // 保存返回地址
            pc = pc + reladdy - 4;  // 跳转目标（减 4 是因为循环末尾会加 4）
            break;
        }
        case 0x67: { // JALR (opcode = 0b1100111)
            // 间接跳转并链接：跳转到 rs1 + 立即数，返回地址存入 rd
            uint32_t imm = ir >> 20;
            int32_t imm_se = imm | ((imm & 0x800) ? 0xfffff000 : 0);
            rval = pc + 4;
            pc = ((REG((ir >> 15) & 0x1f) + imm_se) & ~1) - 4;
            break;
        }
        case 0x63: { // 分支指令 (opcode = 0b1100011)
            // BEQ, BNE, BLT, BGE, BLTU, BGEU
            uint32_t immm4 = ((ir & 0xf00) >> 7) | ((ir & 0x7e000000) >> 20) | ((ir & 0x80) << 4) | ((ir >> 31) << 12);
            if (immm4 & 0x1000)
                immm4 |= 0xffffe000;  // 符号扩展
            int32_t rs1 = REG((ir >> 15) & 0x1f);
            int32_t rs2 = REG((ir >> 20) & 0x1f);
            immm4 = pc + immm4 - 4;
            rdid = 0;
            switch ((ir >> 12) & 0x7) {
            case 0: if (rs1 == rs2) pc = immm4; break;   // BEQ: 相等则跳转
            case 1: if (rs1 != rs2) pc = immm4; break;   // BNE: 不等则跳转
            case 4: if (rs1 < rs2) pc = immm4; break;    // BLT: 有符号小于则跳转
            case 5: if (rs1 >= rs2) pc = immm4; break;   // BGE: 有符号大于等于则跳转
            case 6: if ((uint32_t)rs1 < (uint32_t)rs2) pc = immm4; break;  // BLTU: 无符号小于
            case 7: if ((uint32_t)rs1 >= (uint32_t)rs2) pc = immm4; break; // BGEU: 无符号大于等于
            default:
                trap = (2 + 1);  // 非法指令
            }
            break;
        }
        case 0x03: { // 加载指令 (opcode = 0b0000011)
            // LB, LH, LW, LBU, LHU：从内存加载数据
            uint32_t rs1 = REG((ir >> 15) & 0x1f);
            uint32_t imm = ir >> 20;
            int32_t imm_se = imm | ((imm & 0x800) ? 0xfffff000 : 0);
            uint32_t rsval = rs1 + imm_se;

            rsval -= state->mem_offset;
            if (rsval >= state->mem_size - 3) {
                // 访问内存边界之外 → 可能是 MMIO 区域
                rsval += state->mem_offset;
                if (rsval >= 0x10000000 && rsval < 0x12000000) {
                    // MMIO: 读取定时器
                    if (rsval == 0x1100bffc) rval = CSR(TIMERH);
                    else if (rsval == 0x1100bff8) rval = CSR(TIMERL);
                } else {
                    trap = (5 + 1);  // 加载访问故障
                    rval = rsval;
                }
            } else {
                switch ((ir >> 12) & 0x7) {
                case 0: rval = *(int8_t *)MEM(rsval); break;   // LB: 加载字节（符号扩展）
                case 1: rval = *(int16_t *)MEM(rsval); break;  // LH: 加载半字（符号扩展）
                case 2: rval = *(uint32_t *)MEM(rsval); break; // LW: 加载字
                case 4: rval = *(uint8_t *)MEM(rsval); break;  // LBU: 加载字节（无符号）
                case 5: rval = *(uint16_t *)MEM(rsval); break; // LHU: 加载半字（无符号）
                default: trap = (2 + 1);  // 非法指令
                }
            }
            break;
        }
        case 0x23: { // 存储指令 (opcode = 0b0100011)
            // SB, SH, SW：将数据存储到内存
            uint32_t rs1 = REG((ir >> 15) & 0x1f);
            uint32_t rs2 = REG((ir >> 20) & 0x1f);
            uint32_t addy = ((ir >> 7) & 0x1f) | ((ir & 0xfe000000) >> 20);
            if (addy & 0x800)
                addy |= 0xfffff000;  // 符号扩展
            addy += rs1 - state->mem_offset;
            rdid = 0;

            if (addy >= state->mem_size - 3) {
                // 访问内存边界之外 → 可能是 MMIO 设备区域
                addy += state->mem_offset;
                if (addy >= 0x10000000 && addy < 0x12000000) {
                    // MMIO: CLINT 定时器匹配值 和 SYSCON 系统控制
                    if (addy == 0x11004004) // CLNT: 定时器匹配值高 32 位
                        CSR(TIMERMATCHH) = rs2;
                    else if (addy == 0x11004000) // CLNT: 定时器匹配值低 32 位
                        CSR(TIMERMATCHL) = rs2;
                    else if (addy == 0x11100000) { // SYSCON: 系统控制（重启、关机等）
                        CSR(PC) = pc + 4;
                        return rs2; // 返回退出码，PC 已设置为 SYSCON 之后的指令
                    }
                } else {
                    trap = (7 + 1);  // 存储访问故障
                    rval = addy;
                }
            } else {
                switch ((ir >> 12) & 0x7) {
                case 0: *(uint8_t *)MEM(addy) = rs2; break;   // SB: 存储字节
                case 1: *(uint16_t *)MEM(addy) = rs2; break;  // SH: 存储半字
                case 2: *(uint32_t *)MEM(addy) = rs2; break;  // SW: 存储字
                default:
                    trap = (2 + 1);  // 非法指令
                }
            }
            break;
        }
        case 0x13:   // 立即数运算 (opcode = 0b0010011)
        case 0x33: { // 寄存器运算 (opcode = 0b0110011)
            // 处理算术运算和逻辑运算指令
            uint32_t imm = ir >> 20;
            imm = imm | ((imm & 0x800) ? 0xfffff000 : 0);
            uint32_t rs1 = REG((ir >> 15) & 0x1f);
            uint32_t is_reg = !!(ir & 0x20);  // 0x13(立即数) vs 0x33(寄存器)
            uint32_t rs2 = is_reg ? REG(imm & 0x1f) : imm;

            if (is_reg && (ir & 0x02000000)) {
                // RV32M 乘法扩展：仅有寄存器-寄存器模式
                switch ((ir >> 12) & 7) {
                case 0: rval = rs1 * rs2; break; // MUL: 乘法（低 32 位）
                case 1: rval = ((int64_t)((int32_t)rs1) * (int64_t)((int32_t)rs2)) >> 32; break; // MULH: 有符号乘法高 32 位
                case 2: rval = ((int64_t)((int32_t)rs1) * (uint64_t)rs2) >> 32; break; // MULHSU: 有符号×无符号乘法高 32 位
                case 3: rval = ((uint64_t)rs1 * (uint64_t)rs2) >> 32; break; // MULHU: 无符号乘法高 32 位
                case 4: // DIV: 有符号除法
                    if (rs2 == 0) rval = -1;
                    else rval = ((int32_t)rs1 == INT32_MIN && (int32_t)rs2 == -1) ? rs1 : ((int32_t)rs1 / (int32_t)rs2);
                    break;
                case 5: // DIVU: 无符号除法
                    if (rs2 == 0) rval = 0xffffffff;
                    else rval = rs1 / rs2;
                    break;
                case 6: // REM: 有符号取余
                    if (rs2 == 0) rval = rs1;
                    else rval = ((int32_t)rs1 == INT32_MIN && (int32_t)rs2 == -1) ? 0 : ((uint32_t)((int32_t)rs1 % (int32_t)rs2));
                    break;
                case 7: // REMU: 无符号取余
                    if (rs2 == 0) rval = rs1;
                    else rval = rs1 % rs2;
                    break;
                }
            } else {
                // 基本算术/逻辑运算（立即数版和寄存器版共用）
                switch ((ir >> 12) & 7) {
                case 0: rval = (is_reg && (ir & 0x40000000)) ? (rs1 - rs2) : (rs1 + rs2); break; // ADD/SUB
                case 1: rval = rs1 << (rs2 & 0x1F); break;  // SLL: 逻辑左移
                case 2: rval = (int32_t)rs1 < (int32_t)rs2; break; // SLT: 有符号小于置位
                case 3: rval = rs1 < rs2; break;            // SLTU: 无符号小于置位
                case 4: rval = rs1 ^ rs2; break;            // XOR: 异或
                case 5: rval = (ir & 0x40000000) ? (((int32_t)rs1) >> (rs2 & 0x1F)) : (rs1 >> (rs2 & 0x1F)); break; // SRA/SRL
                case 6: rval = rs1 | rs2; break;            // OR: 或
                case 7: rval = rs1 & rs2; break;            // AND: 与
                }
            }
            break;
        }
        case 0x0f:    //  fences (opcode = 0b0001111)
            rdid = 0; // 本模拟器忽略内存屏障
            break;
        case 0x73: { // Zifencei+Zicsr (opcode = 0b1110011)
            // 处理 CSR 访问、WFI、MRET、ECALL、EBREAK 等
            uint32_t csrno = ir >> 20;       // CSR 编号
            uint32_t microop = (ir >> 12) & 0x7;  // 子操作码
            if ((microop & 3)) { // CSR 读写指令（Zicsr 扩展）
                int rs1imm = (ir >> 15) & 0x1f;
                uint32_t rs1 = REG(rs1imm);
                uint32_t writeval = rs1;

                // 读取 CSR
                switch (csrno) {
                case 0x340: rval = CSR(MSCRATCH); break;
                case 0x305: rval = CSR(MTVEC); break;
                case 0x304: rval = CSR(MIE); break;
                case 0xC00: rval = cycle; break;       // cycle: 周期计数器
                case 0x344: rval = CSR(MIP); break;
                case 0x341: rval = CSR(MEPC); break;
                case 0x300: rval = CSR(MSTATUS); break;
                case 0x342: rval = CSR(MCAUSE); break;
                case 0x343: rval = CSR(MTVAL); break;
                case 0xf11: rval = 0xff0ff0ff; break;  // mvendorid: 制造商 ID
                case 0x301: rval = 0x40401101; break;  // misa: ISA 编码（RV32IMA）
                default: break;
                }

                // 根据微操作类型计算写入值
                switch (microop) {
                case 1: writeval = rs1; break;           // CSRRW
                case 2: writeval = rval | rs1; break;     // CSRRS: 读后置位
                case 3: writeval = rval & ~rs1; break;    // CSRRC: 读后清除
                case 5: writeval = rs1imm; break;         // CSRRWI: 立即数版
                case 6: writeval = rval | rs1imm; break;  // CSRRSI
                case 7: writeval = rval & ~rs1imm; break; // CSRRCI
                }

                // 写入 CSR
                switch (csrno) {
                case 0x340: CSR(MSCRATCH) = writeval; break;
                case 0x305: CSR(MTVEC) = writeval; break;
                case 0x304: CSR(MIE) = writeval; break;
                case 0x344: CSR(MIP) = writeval; break;
                case 0x341: CSR(MEPC) = writeval; break;
                case 0x300: CSR(MSTATUS) = writeval; break;
                case 0x342: CSR(MCAUSE) = writeval; break;
                case 0x343: CSR(MTVAL) = writeval; break;
                default: break;
                }
            } else if (microop == 0x0) { // SYSTEM 类指令 (0b000)
                rdid = 0;
                if (csrno == 0x105) { // WFI: 等待中断
                    CSR(MSTATUS) |= 8;    // 使能中断
                    CSR(EXTRAFLAGS) |= 4; // 标记休眠状态
                    CSR(PC) = pc + 4;
                    return 1;
                } else if (((csrno & 0xff) == 0x02)) { // MRET: 从机器模式陷阱返回
                    uint32_t startmstatus = CSR(MSTATUS);
                    uint32_t startextraflags = CSR(EXTRAFLAGS);
                    CSR(MSTATUS) = ((startmstatus & 0x80) >> 4) | ((startextraflags & 3) << 11) | 0x80;
                    CSR(EXTRAFLAGS) = (startextraflags & ~3) | ((startmstatus >> 11) & 3);
                    pc = CSR(MEPC) - 4;
                } else {
                    switch (csrno) {
                    case 0:
                        trap = (CSR(EXTRAFLAGS) & 3) ? (11 + 1) : (8 + 1);
                        break; // ECALL: 环境调用（U 模式=8，M 模式=11）
                    case 1:
                        trap = (3 + 1);
                        break; // EBREAK: 断点（异常码=3）
                    default:
                        trap = (2 + 1);
                        break; // 非法指令
                    }
                }
            } else
                trap = (2 + 1);  // 非法指令
            break;
        }
        case 0x2f: { // RV32A 原子操作 (opcode = 0b00101111)
            // LR.W, SC.W, AMO*: 原子访存操作
            uint32_t rs1 = REG((ir >> 15) & 0x1f);
            uint32_t rs2 = REG((ir >> 20) & 0x1f);
            uint32_t irmid = (ir >> 27) & 0x1f;

            rs1 -= state->mem_offset;

            // 本模拟器没有实现通过原子操作访问 UART 或 CLINT
            if (rs1 >= state->mem_size - 3) {
                trap = (7 + 1);  // 存储/原子操作访问故障
                rval = rs1 + state->mem_offset;
            } else {
                rval = *(uint32_t *)MEM(rs1);  // 读取原始值

                // 参考了 https://github.com/franzflasch/riscv_em/blob/master/src/core/core.c
                uint32_t dowrite = 1;  // 是否写回
                switch (irmid) {
                case 2: // LR.W (0b00010): 加载预留
                    dowrite = 0;
                    CSR(EXTRAFLAGS) = (CSR(EXTRAFLAGS) & 0x07) | (rs1 << 3);
                    break;
                case 3: // SC.W (0b00011): 条件存储
                    rval = (CSR(EXTRAFLAGS) >> 3 != (rs1 & 0x1fffffff));
                    dowrite = !rval;  // 仅当预留槽位有效时才写入
                    break;
                case 1:  break; // AMOSWAP.W (0b00001): 原子交换
                case 0: rs2 += rval; break; // AMOADD.W (0b00000): 原子加
                case 4: rs2 ^= rval; break; // AMOXOR.W (0b00100): 原子异或
                case 12: rs2 &= rval; break; // AMOAND.W (0b01100): 原子与
                case 8: rs2 |= rval; break;  // AMOOR.W  (0b01000): 原子或
                case 16: rs2 = ((int32_t)rs2 < (int32_t)rval) ? rs2 : rval; break; // AMOMIN.W: 有符号最小值
                case 20: rs2 = ((int32_t)rs2 > (int32_t)rval) ? rs2 : rval; break; // AMOMAX.W: 有符号最大值
                case 24: rs2 = (rs2 < rval) ? rs2 : rval; break;  // AMOMINU.W: 无符号最小值
                case 28: rs2 = (rs2 > rval) ? rs2 : rval; break;  // AMOMAXU.W: 无符号最大值
                default:
                    trap = (2 + 1);
                    dowrite = 0;
                    break; // 不支持的原子操作
                }
                if (dowrite)
                    *(uint32_t *)MEM(rs1) = rs2;
            }
            break;
        }
        default:
            trap = (2 + 1);  // 非法指令
        }

        // ============================================================
        // 6. 如果没有陷阱，将结果写回目标寄存器
        // ============================================================
        if (trap)
            goto cycle_end;

        if (rdid) {
            state->regs[rdid] = rval;
        }
    }

    pc += 4;  // 正常情况：PC 前进到下一条指令

    // ============================================================
    // 7. 陷阱和中断处理（cycle_end）
    // ============================================================
cycle_end:
    if (trap) {
        if (trap & 0x80000000) { // 中断（最高位为 1）
            CSR(MCAUSE) = trap;
            CSR(MTVAL) = 0;
            pc += 4; // PC 应指向中断返回后的位置
        } else { // 异常
            CSR(MCAUSE) = trap - 1;
            CSR(MTVAL) = (trap > 5 && trap <= 8) ? rval : pc;
        }
        CSR(MEPC) = pc;  // 保存异常返回地址
        // 保存旧的中断使能状态到 MPIE
        CSR(MSTATUS) = ((CSR(MSTATUS) & 0x08) << 4) | ((CSR(EXTRAFLAGS) & 3) << 11);
        pc = (CSR(MTVEC) - 4);  // 跳转到陷阱向量

        CSR(EXTRAFLAGS) |= 3;  // 进入机器模式
        trap = 0;
        pc += 4;
    }

    // ============================================================
    // 8. 更新周期计数器和 PC
    // ============================================================
    if (CSR(CYCLEL) > cycle)
        CSR(CYCLEH)++;   // 低 32 位溢出，高位加 1
    CSR(CYCLEL) = cycle;
    CSR(PC) = pc;
    return 0;
}
