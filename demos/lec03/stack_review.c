#include <stdio.h>
static void A(int tmp);
static void C(void) { A(2); }
static void B(void) { C(); }
static void A(int tmp) {
    if (tmp < 2) B();
    printf("%d\n", tmp);
}
int main(void) { A(1); return 0; }
