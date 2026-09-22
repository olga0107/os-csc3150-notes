#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
static void on_int(int signo) {
    (void)signo;
    const char message[] = "caught SIGINT\n";
    (void)write(STDOUT_FILENO, message, sizeof message - 1);
    _exit(1);
}
int main(void) {
    struct sigaction action = {0};
    action.sa_handler = on_int;
    if (sigemptyset(&action.sa_mask) == -1 ||
        sigaction(SIGINT, &action, NULL) == -1) {
        perror("sigaction"); return EXIT_FAILURE;
    }
    puts("ready");
    if (fflush(stdout) == EOF) { perror("fflush"); return EXIT_FAILURE; }
    for (;;) pause();
}
