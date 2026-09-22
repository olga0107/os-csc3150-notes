#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: cpu LABEL\n");
        return EXIT_FAILURE;
    }
    char *str = argv[1];
    while (1) {
        printf("%s\n", str);
        fflush(stdout);  /* Flush this output; scheduling order is still unspecified. */
    }
    return 0;
}
