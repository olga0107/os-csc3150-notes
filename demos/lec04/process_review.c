#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
int main(void) {
    int x = 10;
    printf("before fork: x=%d\n", x);
    if (fflush(stdout) == EOF) { perror("fflush"); return EXIT_FAILURE; }
    pid_t child = fork();
    if (child < 0) { perror("fork"); return EXIT_FAILURE; }
    if (child == 0) {
        ++x;
        printf("child before exec: x=%d\n", x);
        if (fflush(stdout) == EOF) _exit(126);
        char *args[] = {"echo", "child: new program", NULL};
        execv("/bin/echo", args);
        perror("execv");
        _exit(127);
    }
    int status;
    while (waitpid(child, &status, 0) == -1) {
        if (errno == EINTR) continue;
        perror("waitpid");
        return EXIT_FAILURE;
    }
    if (WIFEXITED(status)) {
        printf("parent after wait: x=%d, child exit=%d\n", x, WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status))
        fprintf(stderr, "child terminated by signal %d\n", WTERMSIG(status));
    return EXIT_FAILURE;
}
