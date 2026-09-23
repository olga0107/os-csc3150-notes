#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static void take3(int fd, const char *label) {
    char buf[4] = {0};
    size_t done = 0;
    while (done < 3) {
        ssize_t n = read(fd, buf + done, 3 - done);
        if (n == -1 && errno == EINTR) continue;
        if (n < 0) { perror("read"); exit(EXIT_FAILURE); }
        if (n == 0) { fprintf(stderr, "input must contain at least 9 bytes\n"); exit(EXIT_FAILURE); }
        done += (size_t)n;
    }
    off_t pos = lseek(fd, 0, SEEK_CUR);
    if (pos == (off_t)-1) { perror("lseek"); exit(EXIT_FAILURE); }
    if (printf("%s: %s, offset=%lld\n", label, buf, (long long)pos) < 0 || fflush(stdout) == EOF)
        exit(EXIT_FAILURE);
}
int main(int argc, char *argv[]) {
    if (argc != 2) { fprintf(stderr, "usage: fork_offset INPUT\n"); return EXIT_FAILURE; }
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) { perror("open"); return EXIT_FAILURE; }
    take3(fd, "before fork");
    pid_t pid = fork();
    if (pid == -1) { perror("fork"); close(fd); return EXIT_FAILURE; }
    if (pid == 0) {
        take3(fd, "child");
        if (close(fd) == -1) { perror("child close"); _exit(EXIT_FAILURE); }
        _exit(EXIT_SUCCESS);
    }
    int status;
    pid_t waited;
    do { waited = waitpid(pid, &status, 0); } while (waited == -1 && errno == EINTR);
    if (waited == -1) { perror("waitpid"); close(fd); return EXIT_FAILURE; }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) { close(fd); return EXIT_FAILURE; }
    take3(fd, "parent after child close");
    if (close(fd) == -1) { perror("parent close"); return EXIT_FAILURE; }
    int fresh = open(argv[1], O_RDONLY);
    if (fresh == -1) { perror("reopen"); return EXIT_FAILURE; }
    take3(fresh, "independent open");
    if (close(fresh) == -1) { perror("fresh close"); return EXIT_FAILURE; }
    return EXIT_SUCCESS;
}
