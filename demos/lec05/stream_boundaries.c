#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void require(int ok, const char *message) {
    if (!ok) { fprintf(stderr, "%s\n", message); exit(EXIT_FAILURE); }
}

static FILE *sample(const char *text) {
    FILE *fp = tmpfile();
    require(fp != NULL, "tmpfile failed");
    size_t size = strlen(text);
    require(fwrite(text, 1, size, fp) == size, "write failed");
    require(fseek(fp, 0, SEEK_SET) == 0, "seek failed");
    return fp;
}

int main(void) {
    char buf[8];
    FILE *fp = sample("ABCDE\n");
    require(fgets(buf, 4, fp) != NULL && strcmp(buf, "ABC") == 0, "first chunk");
    printf("fgets 1: ABC, length=%zu\n", strlen(buf));
    require(fgets(buf, 4, fp) != NULL && strcmp(buf, "DE\n") == 0, "second chunk");
    printf("fgets 2: DE\\n, length=%zu\n", strlen(buf));
    require(fgets(buf, 4, fp) == NULL && feof(fp) && !ferror(fp), "expected EOF");
    require(fclose(fp) == 0, "close failed");

    fp = sample("ABCDE");
    size_t count = fread(buf, 2, 3, fp);
    long position = ftell(fp);
    require(count == 2 && position == 5 && feof(fp) && !ferror(fp), "partial element");
    printf("partial: elements=%zu, position=%ld, eof=%d\n", count, position, !!feof(fp));
    require(fclose(fp) == 0, "close failed");

    fp = sample("ABCDEF");
    count = fread(buf, 1, 6, fp);
    require(count == 6 && !ferror(fp), "exact read failed");
    printf("exact: bytes=%zu, eof=%d\n", count, !!feof(fp));
    count = fread(buf, 1, 1, fp);
    require(count == 0 && feof(fp) && !ferror(fp), "EOF probe");
    printf("next: bytes=%zu, eof=%d\n", count, !!feof(fp));
    require(fclose(fp) == 0, "close failed");

    fp = sample("12 34");
    int a = 0, b = 0;
    int assigned = fscanf(fp, "%d%d", &a, &b);
    require(assigned == 2 && a == 12 && b == 34, "formatted input failed");
    printf("formatted: assigned=%d, a=%d, b=%d\n", assigned, a, b);
    require(fclose(fp) == 0, "close failed");
    return EXIT_SUCCESS;
}
