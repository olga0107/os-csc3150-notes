#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int write_all(const char *text) {
    size_t sent = 0, length = strlen(text);
    while (sent < length) {
        ssize_t n = write(STDOUT_FILENO, text + sent, length - sent);
        if (n == -1 && errno == EINTR) continue;
        if (n <= 0) return -1;
        sent += (size_t)n;
    }
    return 0;
}
int main(int argc, char *argv[]) {
    if (argc != 2 || (strcmp(argv[1], "low") && strcmp(argv[1], "line") && strcmp(argv[1], "flush"))) {
        fprintf(stderr, "usage: buffering low|line|flush\n"); return EXIT_FAILURE;
    }
    if (strcmp(argv[1], "low") == 0) {
        if (write_all("Beginning of line ") < 0) return EXIT_FAILURE;
        sleep(1);
        if (write_all("and end of line\n") < 0) return EXIT_FAILURE;
    } else {
        /* Set the policy explicitly so a redirected test is reproducible. */
        if (setvbuf(stdout, NULL, _IOLBF, 0) != 0) return EXIT_FAILURE;
        if (fputs("Beginning of line ", stdout) == EOF) return EXIT_FAILURE;
        if (strcmp(argv[1], "flush") == 0 && fflush(stdout) == EOF) return EXIT_FAILURE;
        sleep(1);
        if (fputs("and end of line\n", stdout) == EOF) return EXIT_FAILURE;
        if (fflush(stdout) == EOF) return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
