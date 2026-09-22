#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <time.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    int *p = malloc(sizeof(int));
    if (p == NULL) { perror("malloc"); return EXIT_FAILURE; }
    printf("(%ld) p: %p\n", (long)getpid(), (void *)p);
    *p = 0;
    while (1) {
        if (*p == INT_MAX) *p = 0;
        *p = *p + 1;
        printf("(%d) p: %d\n", (int)getpid(), *p);
        fflush(stdout);
        const struct timespec delay = {0, 300000000L};
        nanosleep(&delay, NULL);  /* Demonstration pacing, not synchronization. */
    }
    return 0;
}
