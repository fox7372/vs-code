#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<assert.h>

// 引用被测函数
void exchange(int*a,int*b);
void fast_order(int*ptr,int left ,int right);

// 验证数组是否升序
int is_sorted(int* arr, int n) {
    for(int i = 1; i < n; i++) {
        if(arr[i-1] > arr[i]) return 0;
    }
    return 1;
}

// 验证数组包含相同的元素集合
int same_elements(int* a, int* b, int n) {
    int *ca = (int*)calloc(100001, sizeof(int));
    int *cb = (int*)calloc(100001, sizeof(int));
    for(int i = 0; i < n; i++) {
        if(a[i] >= 0 && a[i] <= 100000) ca[a[i]]++;
        if(b[i] >= 0 && b[i] <= 100000) cb[b[i]]++;
    }
    for(int i = 0; i <= 100000; i++) {
        if(ca[i] != cb[i]) return 0;
    }
    free(ca); free(cb);
    return 1;
}

void test_sorted_input() {
    printf("Test 1: already sorted... ");
    int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int copy[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    fast_order(arr, 0, 9);
    assert(is_sorted(arr, 10));
    assert(same_elements(arr, copy, 10));
    printf("PASSED\n");
}

void test_reverse_sorted() {
    printf("Test 2: reverse sorted... ");
    int arr[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    int copy[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    fast_order(arr, 0, 9);
    assert(is_sorted(arr, 10));
    assert(same_elements(arr, copy, 10));
    printf("PASSED\n");
}

void test_duplicates() {
    printf("Test 3: duplicates... ");
    int arr[] = {5, 3, 5, 3, 5, 3, 5, 3, 5, 3};
    int copy[] = {5, 3, 5, 3, 5, 3, 5, 3, 5, 3};
    fast_order(arr, 0, 9);
    assert(is_sorted(arr, 10));
    assert(same_elements(arr, copy, 10));
    printf("PASSED\n");
}

void test_single_element() {
    printf("Test 4: single element... ");
    int arr[] = {42};
    fast_order(arr, 0, 0);
    assert(arr[0] == 42);
    printf("PASSED\n");
}

void test_all_same() {
    printf("Test 5: all same value... ");
    int arr[] = {7, 7, 7, 7, 7, 7, 7, 7, 7, 7};
    fast_order(arr, 0, 9);
    assert(is_sorted(arr, 10));
    printf("PASSED\n");
}

void test_two_elements() {
    printf("Test 6: two elements... ");
    int arr[] = {2, 1};
    fast_order(arr, 0, 1);
    assert(is_sorted(arr, 2));
    printf("PASSED\n");
}

void test_random_large() {
    printf("Test 7: 10000 random elements... ");
    int n = 10000;
    int* arr = (int*)malloc(n * sizeof(int));
    int* copy = (int*)malloc(n * sizeof(int));
    srand(42);
    for(int i = 0; i < n; i++) {
        arr[i] = rand() % 100001;
        copy[i] = arr[i];
    }
    fast_order(arr, 0, n-1);
    assert(is_sorted(arr, n));
    assert(same_elements(arr, copy, n));
    printf("PASSED\n");
    free(arr);
    free(copy);
}

int main() {
    srand(time(NULL));
    test_sorted_input();
    test_reverse_sorted();
    test_duplicates();
    test_single_element();
    test_all_same();
    test_two_elements();
    test_random_large();
    printf("\nAll tests passed!\n");
    return 0;
}
