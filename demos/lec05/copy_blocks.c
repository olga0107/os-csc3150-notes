#include <stdio.h>
#include <stdlib.h>
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: copy_blocks INPUT OUTPUT (different files)\n");
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
    char buffer[BUFFER_SIZE];
    size_t length;
    int failed = 0;
    while ((length = fread(buffer, sizeof(char), BUFFER_SIZE, input)) > 0) {
        printf("read = %zu\n", length);
        if (fwrite(buffer, sizeof(char), length, output) != length) {
            perror("write output");
            failed = 1;
            break;
        }
    }
    if (ferror(input)) { perror("read input"); failed = 1; }
    if (fclose(input) == EOF) { perror("close input"); failed = 1; }
    if (fclose(output) == EOF) { perror("close output"); failed = 1; }
    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
