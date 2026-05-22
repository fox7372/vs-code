// fish.c — 状态机驱动的并发同步 demo
// 多个线程按状态转移规则打印 < > _ ，每步只能一个字符通过。

#include "thread.h"
#include "thread-sync.h"

#define LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

enum { A = 1, B, C, D, E, F, };

// 状态转移规则: (当前状态, 输入字符) → 下一状态
struct rule {
  int from, ch, to;
} rules[] = {
    {A, '<', B},
    {B, '>', C},
    {C, '<', D},
    {A, '>', E},
    {E, '<', F},
    {F, '>', D},
    {D, '_', A},
};

int current = A;     // 当前状态
int quota = 1;       // 配额: 每次只允许一个字符通过

mutex_t lk = MUTEX_INIT();
cond_t cv = COND_INIT();

// 查询当前状态下，输入字符 ch 是否能触发转移
int next(char ch) {
    for (int i = 0; i < LENGTH(rules); i++) {
        struct rule *rule = &rules[i];
        if (rule->from == current && rule->ch == ch) {
          return rule->to;
        }
    }
    return 0;  // 未找到匹配 → 不可打印
}

// 检查能否打印 ch: 有对应的状态转移 + 配额充足
static int can_print(char ch) {
    return next(ch) != 0 && quota > 0;
}

// 打印前的同步: 等条件满足后消耗配额
void fish_before(char ch) {
    mutex_lock(&lk);
    while (!can_print(ch)) {          // 不满足条件就等
        cond_wait(&cv, &lk);
    }
    quota--;                          // 消耗一个配额
    mutex_unlock(&lk);
}

// 打印后的同步: 恢复配额、推进状态、唤醒等待线程
void fish_after(char ch) {
    mutex_lock(&lk);
    quota++;                          // 恢复一个配额
    current = next(ch);               // 推进到下一状态
    assert(current);                  // 必须找到有效转移
    cond_broadcast(&cv);              // 唤醒所有等待线程重新竞争
    mutex_unlock(&lk);
}

// 角色表: '.' 占位，5个 <, 4个 >, 2个 _
const char roles[] = ".<<<<<>>>>___";

void fish_thread(int id) {
    char role = roles[id];            // 每个线程持有一个固定字符
    while (1) {
        fish_before(role);
        putchar(role);                // 打印角色字符（受 quota 保护，无需额外锁）
        fish_after(role);
    }
}

int main() {
    setbuf(stdout, NULL);             // 关闭缓冲区，即时输出
    for (int i = 0; i < strlen(roles); i++)
        spawn(fish_thread);           // 为每个角色创建一个线程
}
