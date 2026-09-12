#include <stdio.h>
#include <stdlib.h>

int global_var = 10;                 /* static data segment */

int main() {
    int stack_var = 20;              /* stack segment */
    int *heap_var = malloc(sizeof(int));   /* heap segment */
    *heap_var = 40;

    printf("code   (main):    %p\n", (void*)main);
    printf("data   (global):  %p\n", (void*)&global_var);
    printf("heap   (malloc):  %p\n", (void*)heap_var);
    printf("stack  (local):   %p\n", (void*)&stack_var);

    free(heap_var);
    return 0;
}
