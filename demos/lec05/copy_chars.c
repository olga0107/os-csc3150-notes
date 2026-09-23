#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: copy_chars INPUT OUTPUT (different files)\n");
        return EXIT_FAILURE;
    }
    FILE *input = fopen(argv[1], "rb");
    if (input == NULL) { perror("open input"); return EXIT_FAILURE; }
    FILE *output = fopen(argv[2], "wb");
    if (output == NULL) {
        perror("open output");
        fclose(input);
        return EXIT_FAILURE;
    }
    int failed = 0;
    int c = fgetc(input);
    while (c != EOF) {
        if (fputc(c, output) == EOF) {
            perror("write output");
            failed = 1;
            break;
        }
        c = fgetc(input);
    }
    if (ferror(input)) { perror("read input"); failed = 1; }
    if (fclose(input) == EOF) { perror("close input"); failed = 1; }
    if (fclose(output) == EOF) { perror("close output"); failed = 1; }
    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
