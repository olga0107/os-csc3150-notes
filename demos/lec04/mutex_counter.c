#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int counter = 0;
enum { ITERATIONS = 100000, WORKERS = 2 };
static void check(int rc, const char *operation) {
    if (rc != 0) {
        fprintf(stderr, "%s: %s\n", operation, strerror(rc));
        exit(EXIT_FAILURE);
    }
}
static void *worker(void *arg) {
    (void)arg;
    for (int i = 0; i < ITERATIONS; ++i) {
        check(pthread_mutex_lock(&mutex), "pthread_mutex_lock");
        ++counter;
        check(pthread_mutex_unlock(&mutex), "pthread_mutex_unlock");
    }
    return NULL;
}
int main(void) {
    pthread_t threads[WORKERS];
    for (int i = 0; i < WORKERS; ++i)
        check(pthread_create(&threads[i], NULL, worker, NULL), "pthread_create");
    for (int i = 0; i < WORKERS; ++i)
        check(pthread_join(threads[i], NULL), "pthread_join");
    printf("counter = %d (expected %d)\n", counter, WORKERS * ITERATIONS);
    check(pthread_mutex_destroy(&mutex), "pthread_mutex_destroy");
    return counter == WORKERS * ITERATIONS ? EXIT_SUCCESS : EXIT_FAILURE;
}
