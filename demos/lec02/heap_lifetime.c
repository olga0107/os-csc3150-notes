#include <stdio.h>
#include <stdlib.h>
static int *make_number(void) {
    int *p = malloc(sizeof *p);
    if (p != NULL) *p = 42;
    return p;
}
int main(void) {
    int *result = make_number();
    if (result == NULL) {
        fputs("allocation failed\n", stderr);
        return EXIT_FAILURE;
    }
    printf("after function returns: %d\n", *result);
    free(result);
    result = NULL;
    puts("released; result reset to NULL");
    return 0;
}
