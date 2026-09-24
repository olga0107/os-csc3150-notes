#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    int p[2];
    if (pipe(p) == -1) { perror("pipe"); return 1; }
    pid_t pid = fork();
    if (pid == -1) { perror("fork"); return 1; }
    if (pid == 0) {
        close(p[1]);
        char buf[3]; /* Deliberately smaller than the message. */
        for (;;) {
            ssize_t n = read(p[0], buf, sizeof buf);
            if (n > 0) printf("child read %zd bytes: %.*s\n", n, (int)n, buf);
            else if (n == 0) { puts("child read 0: EOF"); break; }
            else if (errno != EINTR) { perror("read"); return 1; }
        }
        close(p[0]);
        return 0;
    }
    close(p[0]);
    const char msg[] = "hello";
    size_t sent = 0;
    while (sent < sizeof msg - 1) {
        ssize_t n = write(p[1], msg + sent, sizeof msg - 1 - sent);
        if (n > 0) sent += (size_t)n;
        else if (n == -1 && errno == EINTR) continue;
        else { perror("write"); close(p[1]); return 1; }
    }
    close(p[1]); /* Reader can see EOF after buffered bytes are consumed. */
    int status;
    while (waitpid(pid, &status, 0) == -1) {
        if (errno != EINTR) { perror("waitpid"); return 1; }
    }
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}
