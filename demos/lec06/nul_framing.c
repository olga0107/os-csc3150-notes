#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* 1 = message, 0 = clean EOF, -1 = I/O error,
   -2 = EOF inside message, -3 = message exceeds buffer. */
int read_nul_message(int fd, char *buf, size_t cap) {
    if (cap == 0) { errno = EINVAL; return -1; }
    size_t used = 0;
    buf[0] = '\0';
    for (;;) {
        char c;
        ssize_t n = read(fd, &c, 1);
        if (n == 1) {
            if (c == '\0') return 1;
            if (used + 1 >= cap) return -3;
            buf[used++] = c;
            buf[used] = '\0';
        } else if (n == 0) {
            return used == 0 ? 0 : -2;
        } else if (errno != EINTR) {
            return -1;
        }
    }
}

static int feed(int p[2], const char *data, size_t len) {
    if (pipe(p) == -1) return -1;
    /* Inputs in this demo are tiny; always check the observed byte count. */
    if (write(p[1], data, len) != (ssize_t)len) {
        close(p[0]); close(p[1]); return -1;
    }
    return close(p[1]);
}

int main(void) {
    int p[2], rc;
    char buf[16];
    const char pair[] = "hello\0next\0";
    if (feed(p, pair, sizeof pair - 1) == -1) return 1;
    rc = read_nul_message(p[0], buf, sizeof buf);
    if (rc != 1 || strcmp(buf, "hello")) return 1;
    printf("message 1: rc=%d, text=%s\n", rc, buf);
    rc = read_nul_message(p[0], buf, sizeof buf);
    if (rc != 1 || strcmp(buf, "next")) return 1;
    printf("message 2: rc=%d, text=%s\n", rc, buf);
    rc = read_nul_message(p[0], buf, sizeof buf);
    if (rc != 0) return 1;
    printf("after messages: rc=%d (clean EOF)\n", rc);
    close(p[0]);
    if (feed(p, "abc", 3) == -1) return 1;
    rc = read_nul_message(p[0], buf, sizeof buf);
    if (rc != -2) return 1;
    printf("missing terminator: rc=%d\n", rc);
    close(p[0]);
    if (feed(p, "long\0", 5) == -1) return 1;
    rc = read_nul_message(p[0], buf, 4);
    if (rc != -3) return 1;
    printf("capacity exceeded: rc=%d\n", rc);
    close(p[0]);
    return 0;
}
