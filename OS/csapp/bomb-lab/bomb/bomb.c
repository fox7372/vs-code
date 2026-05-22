/***************************************************************************
 * 邪恶博士的阴险炸弹，版本 1.1
 * 版权所有 2011，邪恶集团。保留所有权利。
 *
 * 许可证：
 *
 * 邪恶集团（以下简称"行凶者"）特此授予您（以下简称"受害者"）
 * 使用本炸弹的明确许可。此许可证有时间限制，有效期至受害者死亡。
 * 行凶者对任何损害、挫折、精神错乱、金鱼眼、腕管综合征、失眠或
 * 对受害者的其他伤害不承担任何责任。除非行凶者想要邀功。
 * 受害者不得将本炸弹源代码分发给行凶者的任何敌人。
 * 受害者不得调试、逆向工程、运行"strings"、反编译、解密或使用
 * 任何其他技术来获取知识和拆除炸弹。
 * 处理本程序时不得穿戴防弹衣。
 * 在法律禁止炸弹的地区，本许可证无效。
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "support.h"
#include "phases.h"

/*
 * 给自己备注：记得删除这个文件，这样我的受害者就不知道发生了什么，
 * 让他们在壮观邪恶的爆炸中灰飞烟灭。—— 邪恶博士
 */

FILE *infile;

int main(int argc, char *argv[])
{
    char *input;

    /* 给自己备注：记得把这个炸弹移植到 Windows，并给它做个华丽的 GUI。 */

    /* 无参数运行时，炸弹从标准输入读取输入行。 */
    if (argc == 1) {
	infile = stdin;
    }

    /* 带一个参数 <file> 运行时，炸弹从 <file> 读取直到文件结束，
     * 然后切换到标准输入。这样，每拆除一个阶段，
     * 你可以把拆除字符串添加到 <file> 中，避免重新输入。 */
    else if (argc == 2) {
	if (!(infile = fopen(argv[1], "r"))) {
	    printf("%s: Error: Couldn't open %s\n", argv[0], argv[1]);
	    exit(8);
	}
    }

    /* 不允许带超过一个命令行参数调用炸弹。 */
    else {
	printf("Usage: %s [<input_file>]\n", argv[0]);
	exit(8);
    }

    /* 做各种秘密操作，让炸弹更难拆除。 */
    initialize_bomb();

    printf("Welcome to my fiendish little bomb. You have 6 phases with\n");
    printf("which to blow yourself up. Have a nice day!\n");

    /* 嗯... 六个阶段肯定比一个阶段更安全！ */
    input = read_line();             /* 获取输入                         */
    phase_1(input);                  /* 运行阶段                         */
    phase_defused();                 /* 该死！他们居然解开了！
				      * 让我知道他们是怎么做到的。 */
    printf("Phase 1 defused. How about the next one?\n");

    /* 第二阶段更难了。没人能搞清楚怎么拆除这个... */
    input = read_line();
    phase_2(input);
    phase_defused();
    printf("That's number 2.  Keep going!\n");

    /* 我猜这到目前为止太简单了。来点更复杂的代码迷惑他们。 */
    input = read_line();
    phase_3(input);
    phase_defused();
    printf("Halfway there!\n");
 
    /* 哦是吗？你的数学怎么样？试试这个棘手的问题！ */
    input = read_line();
    phase_4(input);
    phase_defused();
    printf("So you got that one.  Try this one.\n");

    /* 在内存中一圈又一圈，我们在哪停下，炸弹就在哪爆炸！ */
    input = read_line();
    phase_5(input);
    phase_defused();
    printf("Good work!  On to the next...\n");

    /* 这一阶段永远不会被用到，因为没人能通过前面的阶段。
     * 但以防万一，让这个格外困难。 */
    input = read_line();
    phase_6(input);
    phase_defused();

    /* 哇，他们成功了！但是不是少了什么...？也许
     * 他们漏掉了什么？呜哈哈哈！ */

    return 0;
}
