#include <stdio.h>
static void by_value(int n) { n = 42; (void)n; }
static void by_pointer(int *p) { *p = 42; }
static void redirect(int **out, int *target) { *out = target; }
int main(void) {
    int x = 7, y = 99;
    int *p = &x;
    by_value(x);
    printf("after value: %d\n", x);
    by_pointer(&x);
    printf("after pointer: %d\n", x);
    redirect(&p, &y);
    printf("redirected: %d\n", *p);
    return 0;
}
