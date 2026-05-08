#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========== 二叉树节点 ========== */
typedef struct tree_n {
    int data;
    struct tree_n *left, *right;
} tree_n;

tree_n* creat_tnode(int data) {
    tree_n* n = (tree_n*)malloc(sizeof(tree_n));
    n->data = data;
    n->left = n->right = NULL;
    return n;
}

void freetree(tree_n* p) {
    if (!p) return;
    freetree(p->left);
    freetree(p->right);
    free(p);
}

int insert_tree(int data, tree_n* root) {
    if (root->data > data) {
        if (!root->left) { root->left = creat_tnode(data); return 0; }
        return insert_tree(data, root->left);
    } else if (root->data < data) {
        if (!root->right) { root->right = creat_tnode(data); return 0; }
        return insert_tree(data, root->right);
    }
    return -1;
}

/* ========== 画屏缓冲区 ========== */
typedef struct {
    char** buf;
    int rows, cols;
} Canvas;

Canvas canvas_create(int rows, int cols) {
    Canvas c = { NULL, rows, cols };
    c.buf = malloc(rows * sizeof(char*));
    for (int r = 0; r < rows; r++) {
        c.buf[r] = malloc((size_t)cols + 1);
        memset(c.buf[r], ' ', (size_t)cols);
        c.buf[r][cols] = '\0';
    }
    return c;
}

void canvas_free(Canvas c) {
    for (int r = 0; r < c.rows; r++) free(c.buf[r]);
    free(c.buf);
}

void canvas_set(Canvas c, int r, int col, char ch) {
    if (r >= 0 && r < c.rows && col >= 0 && col < c.cols)
        c.buf[r][col] = ch;
}

/* 写两位数 */
void canvas_write_num(Canvas c, int r, int col, int val) {
    if (val < 10) {
        canvas_set(c, r, col, '0' + val);
    } else {
        canvas_set(c, r, col, '0' + val / 10);
        canvas_set(c, r, col + 1, '0' + val % 10);
    }
}

void canvas_print(Canvas c) {
    for (int r = 0; r < c.rows; r++) {
        /* 去掉尾部空白 */
        int end = c.cols;
        while (end > 0 && c.buf[r][end - 1] == ' ') end--;
        c.buf[r][end] = '\0';
        printf("%s\n", c.buf[r]);
    }
}

/* ========== 水平树（带框线） ========== */
static void print_horiz_aux(tree_n* root, const char* prefix, int is_last) {
    if (!root) return;
    printf("%s%s── %d\n", prefix, is_last ? "└" : "├", root->data);

    char child_pre[1024];
    snprintf(child_pre, sizeof(child_pre), "%s%s",
             prefix, is_last ? "    " : "│   ");

    if (root->left || root->right) {
        print_horiz_aux(root->left,  child_pre, root->right == NULL);
        print_horiz_aux(root->right, child_pre, 1);
    }
}

void print_horizontal(tree_n* root) {
    if (!root) { printf("(empty)\n"); return; }
    printf("%d\n", root->data);
    if (root->left || root->right) {
        char prefix[1024] = "";
        print_horiz_aux(root->left,  prefix, root->right == NULL);
        print_horiz_aux(root->right, prefix, 1);
    }
}

/* ========== 垂直树（俯视图，基于画屏） ========== */

/* 树高 */
static int tree_h(tree_n* r) {
    if (!r) return 0;
    int l = tree_h(r->left), r2 = tree_h(r->right);
    return (l > r2 ? l : r2) + 1;
}

/* 中序遍历给每个值分配列号（写到 col_of[] 映射表） */
/* 值范围假设在 0~999 以内 */
static void assign_cols(tree_n* r, int* col_of, int* next) {
    if (!r) return;
    assign_cols(r->left, col_of, next);
    col_of[r->data] = (*next)++;
    assign_cols(r->right, col_of, next);
}

/* 递归绘制节点和连线 */
static void draw_tree(tree_n* r, int depth, int* col_of, Canvas c) {
    if (!r) return;

    int col_idx = col_of[r->data];          /* 中序索引 0,1,2,... */
    int cell_w = 6;                          /* 每列字符宽度 */
    int cx = col_idx * cell_w + cell_w / 2;  /* 节点中心 x */
    int row = depth * 2;                     /* 节点所在 y */

    /* 写节点值 */
    canvas_write_num(c, row, cx - 1, r->data);

    /* 先画子节点（确保子节点先写入画屏，不过顺序不影响最终结果） */
    draw_tree(r->left,  depth + 1, col_of, c);
    draw_tree(r->right, depth + 1, col_of, c);

    /* 画到子节点的连线 */
    int line_row = row + 1;  /* 连线行 */

    if (r->left) {
        int l_idx = col_of[r->left->data];
        int lx = l_idx * cell_w + cell_w / 2;
        int slash_col = (cx + lx) / 2;

        /* 从 lx 往右到 slash_col 画 '_'，slash_col 处画 '/' */
        for (int x = lx + 1; x < slash_col; x++)
            canvas_set(c, line_row, x, '_');
        canvas_set(c, line_row, slash_col, '/');
    }

    if (r->right) {
        int r_idx = col_of[r->right->data];
        int rx = r_idx * cell_w + cell_w / 2;
        int slash_col = (cx + rx + 1) / 2;  /* 偏右一点，配合 \ 方向 */

        canvas_set(c, line_row, slash_col, '\\');
        for (int x = slash_col + 1; x < rx; x++)
            canvas_set(c, line_row, x, '_');
    }
}

void print_vertical(tree_n* root) {
    if (!root) return;

    int col_of[1024] = {0};
    int next = 0;
    assign_cols(root, col_of, &next);

    int n_nodes = next;
    int height = tree_h(root);
    int cell_w = 6;

    int canvas_rows = height * 2 - 1;          /* 节点行 + 连线行交替 */
    int canvas_cols = n_nodes * cell_w + 4;     /* 留点边 */

    Canvas c = canvas_create(canvas_rows, canvas_cols);
    draw_tree(root, 0, col_of, c);
    canvas_print(c);
    canvas_free(c);
}

/* ========== 主函数 ========== */
int main() {
    int arr[] = {31, 45, 36,14, 52, 42, 6, 21, 73, 47, 26, 37, 33, 8};
    int n = sizeof(arr) / sizeof(arr[0]);

    tree_n* root = creat_tnode(arr[0]);
    for (int i = 1; i < n; i++)
        insert_tree(arr[i], root);

    printf("================ 水平树 ================\n");
    print_horizontal(root);

    printf("\n=========== 垂直树（俯视图） ===========\n\n");
    print_vertical(root);

    printf("\n");
    freetree(root);
    return 0;
}
