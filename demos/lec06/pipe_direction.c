#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    int p[2];
    if (pipe(p) == -1) { perror("pipe"); return 1; }
    errno = 0;
    ssize_t n = write(p[0], "X", 1);
    int err = errno;
    printf("write(p[0]) = %zd", n);
    if (n == -1) printf(", errno = %d (%s)", err, strerror(err));
    putchar('\n');
    /* Only read the opposite end if the write actually succeeded. */
    if (n == 1) {
        char c;
        n = read(p[1], &c, 1);
        printf("read(p[1]) = %zd\n", n);
    }
    close(p[0]); close(p[1]);
    return 0;
}
