#include <stdio.h>
#include <stdlib.h>

static void show(FILE *fp, long offset, int whence, const char *label) {
    if (fseek(fp, offset, whence) != 0) { perror("fseek"); exit(EXIT_FAILURE); }
    long before = ftell(fp);
    if (before == -1L) { perror("ftell"); exit(EXIT_FAILURE); }
    int c = fgetc(fp);
    if (c == EOF) { fprintf(stderr, "expected a byte\n"); exit(EXIT_FAILURE); }
    long after = ftell(fp);
    if (after == -1L) { perror("ftell"); exit(EXIT_FAILURE); }
    printf("%s: before=%ld, char=%c, after=%ld\n", label, before, c, after);
}
int main(void) {
    FILE *fp = tmpfile();
    if (fp == NULL) { perror("tmpfile"); return EXIT_FAILURE; }
    if (fwrite("ABCDE", 1, 5, fp) != 5) { perror("fwrite"); fclose(fp); return EXIT_FAILURE; }
    show(fp, 2, SEEK_SET, "SET +2");
    show(fp, -1, SEEK_CUR, "CUR -1");
    show(fp, -1, SEEK_END, "END -1");
    if (fclose(fp) == EOF) { perror("fclose"); return EXIT_FAILURE; }
    return EXIT_SUCCESS;
}
