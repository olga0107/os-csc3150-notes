#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    char *str = argv[1];
    while (1) {
        printf("%s\n", str);
        fflush(stdout);  /* unbuffer so we can watch the interleaving live */
    }
    return 0;
}
