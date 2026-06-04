#ifndef __CONFIG_H_
#define __CONFIG_H_

/*
 * config.h - malloc lab 配置文件
 *
 * Copyright (c) 2002, R. Bryant and D. O'Hallaron, All rights reserved.
 * May not be used, modified, or copied without permission.
 */

/*
 * 默认 tracefile 路径。运行时可用 -t 参数覆盖。
 */
#define TRACEDIR "/afs/cs/project/ics2/im/labs/malloclab/traces/"

/*
 * 默认 tracefile 列表。修改此列表可增删测试 trace。
 * 例如，如果不想让学生实现 realloc，可删除最后两个 trace。
 */
#define DEFAULT_TRACEFILES \
  "amptjp-bal.rep",\
  "cccp-bal.rep",\
  "cp-decl-bal.rep",\
  "expr-bal.rep",\
  "coalescing-bal.rep",\
  "random-bal.rep",\
  "random2-bal.rep",\
  "binary-bal.rep",\
  "binary2-bal.rep",\
  "realloc-bal.rep",\
  "realloc2-bal.rep"

/*
 * libc malloc 在参考系统上的预估性能。
 * 用于限制吞吐量对性能指数的贡献上限：
 * 一旦学生超过 AVG_LIBC_THRUPUT，吞吐量不再加分。
 * 防止学生构建极快但极蠢的 malloc。
 */
#define AVG_LIBC_THRUPUT      600E3  /* 600 Kops/秒 */

/*
 * 空间利用率（UTIL_WEIGHT）和吞吐量（1 - UTIL_WEIGHT）对性能指数的贡献权重。
 */
#define UTIL_WEIGHT .60

/*
 * 对齐要求（4 或 8 字节）
 */
#define ALIGNMENT 8

/*
 * 最大堆大小（字节）
 */
#define MAX_HEAP (20*(1<<20))  /* 20 MB */

/*****************************************************************************
 * 将下面其中一个 USE_xxx 设为 1，选择计时方式
 *****************************************************************************/
#define USE_FCYC   0   /* 周期计数器 + K-best 方案（仅 x86 & Alpha） */
#define USE_ITIMER 0   /* 间隔计时器（任意 Unix） */
#define USE_GETTOD 1   /* gettimeofday（任意 Unix） */

#endif /* __CONFIG_H_ */
