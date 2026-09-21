#include <stdio.h>

void try_change(int x)  { x = 42; }      // 只拿到复印件
void real_change(int *x) { *x = 42; }    // 拿到门牌号，上门修改

void give_me_a_pointer(int **pp) {       // 双指针登场
    static int treasure = 99;
    *pp = &treasure;                     // 把"宝藏的门牌号"放进 *pp
}

int main() {
    int a = 0;
    try_change(a);
    printf("try_change 之后: a = %d   <- 复印件改了，原件没变\n", a);
    real_change(&a);
    printf("real_change 之后: a = %d  <- 传了地址，真的被改了\n", a);

    int *p = NULL;
    give_me_a_pointer(&p);
    printf("give_me_a_pointer 之后: p 指向 %d <- 函数把指针放进了我兜里\n", *p);
    return 0;
}
