// SPDX-License-Identifier: MIT
// Mini libc — demo: shows every facility provided by this mini libc.

#include "libc.h"

int main(int argc, char **argv, char **envp) {
    (void)envp; // available via the global 'environ'
    // 1. argv
    printf("=== 1. Command-line args ===\n");
    for (int i = 0; i < argc; i++)
        printf("  argv[%d] = %s\n", i, argv[i]);

    // 2. environ
    printf("\n=== 2. Environment (first 3) ===\n");
    for (char **e = environ; *e && e - environ < 3; e++)
        printf("  %s\n", *e);

    // 3. printf format specifiers
    printf("\n=== 3. printf formats ===\n");
    printf("  signed decimal:   %d\n", -42);
    printf("  unsigned decimal: %u\n", 42u);
    printf("  hex (lower):      0x%x\n", 255);
    printf("  string:           %s\n", "hello from mini-libc!");
    printf("  char:             '%c'\n", 'A');
    printf("  pointer:          %p\n", (void*)0xdeadbeef);
    printf("  literal percent:  %%\n");

    // 4. string operations
    printf("\n=== 4. String functions ===\n");
    printf("  strlen(\"hello\")        = %d\n", (int)strlen("hello"));
    printf("  strcmp(\"abc\", \"abc\")  = %d\n", strcmp("abc", "abc"));
    printf("  strcmp(\"abc\", \"abd\")  = %d\n", strcmp("abc", "abd"));
    printf("  strcmp(\"abd\", \"abc\")  = %d\n", strcmp("abd", "abc"));

    char buf[32];
    memset(buf, 0, 32);
    memcpy(buf, "memcpy + memset work", 20);
    printf("  %s\n", buf);

    memmove(buf + 7, buf, 6);
    printf("  after memmove:      %s\n", buf);

    printf("  strchr(\"foobar\", 'b') = %s\n", strchr("foobar", 'b'));
    printf("  memcmp(\"abc\",\"abc\",3) = %d\n", memcmp("abc","abc",3));
    printf("  memcmp(\"abc\",\"abd\",3) = %d\n", memcmp("abc","abd",3));

    // 5. atoi / strtol
    printf("\n=== 5. atoi / strtol ===\n");
    printf("  atoi(\"123\")      = %d\n", atoi("123"));
    printf("  atoi(\"-42\")      = %d\n", atoi("-42"));
    printf("  strtol(\"0xff\",NULL,0) = %ld\n", strtol("0xff",NULL,0));
    printf("  strtol(\" 77\",NULL,8)  = %ld\n", strtol(" 77",NULL,8));
    printf("  strtol(\"101\",NULL,2)  = %ld\n", strtol("101",NULL,2));

    // 6. malloc
    printf("\n=== 6. malloc ===\n");
    int n = 10;
    int *arr = malloc(n * sizeof(int));
    if (!arr) {
        dprintf(2, "malloc failed!\n");
        return 1;
    }
    for (int i = 0; i < n; i++) arr[i] = i * 100;
    for (int i = 0; i < n; i++) printf("  arr[%d] = %d\n", i, arr[i]);
    free(arr);

    // 7. ctype
    printf("\n=== 7. ctype ===\n");
    printf("  isalpha('A') = %d  isdigit('3') = %d  isspace(' ') = %d\n",
           isalpha('A'), isdigit('3'), isspace(' '));

    printf("\n=== All tests passed! ===\n");
    return 0;
}
