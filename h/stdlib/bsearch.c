#include <stdio.h>
#include <stdlib.h>
 
// 比较函数，用于 bsearch 查找
int cmpfunc(const void * a, const void * b) {
    return (*(int*)a - *(int*)b);
}
 
int main() {
    int values[] = { 5, 20, 29, 32, 63 };
    int key = 32;
    int *item;
 
    // 计算数组长度
    size_t array_size = sizeof(values) / sizeof(values[0]);
 
    // 使用 bsearch 在数组中查找值 32
    item = (int*) bsearch(&key, values, array_size, sizeof(int), cmpfunc);
 
    // 检查查找结果并输出
    if (item != NULL) {
        printf("Found item = %d\n", *item);
    } else {
        printf("Item = %d could not be found\n", key);
    }
 
    return 0;
}