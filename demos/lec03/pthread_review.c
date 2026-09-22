#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct job { int input; int output; };
static void check(int rc, const char *operation) {
    if (rc != 0) {
        fprintf(stderr, "%s: %s\n", operation, strerror(rc));
        exit(EXIT_FAILURE);
    }
}
static void *worker(void *arg) {
    struct job *job = arg;
    job->output = job->input * job->input;
    return job;
}
int main(void) {
    struct job jobs[2] = {{3, 0}, {4, 0}};
    pthread_t tids[2];
    for (int i = 0; i < 2; ++i)
        check(pthread_create(&tids[i], NULL, worker, &jobs[i]), "pthread_create");
    for (int i = 0; i < 2; ++i) {
        void *result = NULL;
        check(pthread_join(tids[i], &result), "pthread_join");
        struct job *job = result;
        printf("%d squared = %d\n", job->input, job->output);
    }
    return 0;
}
