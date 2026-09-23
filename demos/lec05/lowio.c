#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: lowio INPUT\n");
        return EXIT_FAILURE;
    }
    char buf[1000];
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) { perror("open"); return EXIT_FAILURE; }
    ssize_t rd;
    do { rd = read(fd, buf, sizeof buf); } while (rd == -1 && errno == EINTR);
    if (rd == -1) {
        perror("read"); close(fd); return EXIT_FAILURE;
    }
    if (close(fd) == -1) { perror("close"); return EXIT_FAILURE; }
    size_t sent = 0;
    while (sent < (size_t)rd) {
        ssize_t wr = write(STDOUT_FILENO, buf + sent, (size_t)rd - sent);
        if (wr == -1 && errno == EINTR) continue;
        if (wr <= 0) {
            if (wr == 0) errno = EIO;
            perror("write"); return EXIT_FAILURE;
        }
        sent += (size_t)wr;
    }
    return EXIT_SUCCESS;
}
