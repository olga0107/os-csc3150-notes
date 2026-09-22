#include <stdio.h>
int main(int argc, char *argv[]) {
    printf("argc = %d\n", argc);
    for (int i = 0; i < argc; ++i)
        printf("argv[%d] = %s\n", i, argv[i]);
    printf("argv[argc] is NULL: %s\n", argv[argc] == NULL ? "yes" : "no");
    return 0;
}
