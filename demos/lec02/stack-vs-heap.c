#include <stdio.h>
#include <stdlib.h>

void level3() {
    int c = 3;
    printf("level3 的局部变量: %p\n", (void*)&c);
}

void level2() {
    int b = 2;
    printf("level2 的局部变量: %p\n", (void*)&b);
    level3();
}

int main() {
    int a = 1;
    printf("main   的局部变量: %p\n", (void*)&a);
    level2();

    printf("\n");

    int *h1 = malloc(100);
    int *h2 = malloc(100);
    printf("h1 = malloc(100):  %p\n", (void*)h1);
    printf("h2 = malloc(100):  %p\n", (void*)h2);

    free(h1);                      /* 把 h1 的柜子还回去 */
    int *h3 = malloc(100);
    printf("h3 = malloc(100):  %p\n", (void*)h3);

    free(h2);
    free(h3);
    return 0;
}
