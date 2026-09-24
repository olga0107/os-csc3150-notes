#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

static void die(const char *operation) { perror(operation); exit(1); }

static int write_all(int fd, const char *buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = write(fd, buf + sent, len - sent);
        if (n > 0) sent += (size_t)n;
        else if (n == -1 && errno == EINTR) continue;
        else { if (n == 0) errno = EIO; return -1; }
    }
    return 0;
}

static int open_socket(const char *host, const char *port, int server) {
    struct addrinfo hints = {0}, *list;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int rc = getaddrinfo(host, port, &hints, &list);
    if (rc != 0) { fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc)); exit(1); }
    int fd = -1, saved = 0;
    for (struct addrinfo *a = list; a != NULL; a = a->ai_next) {
        fd = socket(a->ai_family, a->ai_socktype, a->ai_protocol);
        if (fd == -1) { saved = errno; continue; }
        if (server) {
            int yes = 1;
            if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
                saved = errno; close(fd); fd = -1; continue;
            }
            rc = bind(fd, a->ai_addr, a->ai_addrlen);
        } else rc = connect(fd, a->ai_addr, a->ai_addrlen);
        if (rc == 0) break;
        saved = errno; close(fd); fd = -1;
    }
    freeaddrinfo(list);
    if (fd == -1) { errno = saved; die(server ? "bind/socket" : "connect/socket"); }
    return fd;
}

static void serve_client(int fd) {
    char buf[128];
    for (;;) {
        ssize_t n = read(fd, buf, sizeof buf);
        if (n > 0) {
            printf("server read %zd bytes\n", n);
            if (write_all(fd, buf, (size_t)n) == -1) { perror("write"); return; }
        } else if (n == 0) { puts("client EOF"); return; }
        else if (errno != EINTR) { perror("read"); return; }
    }
}

static void run_client(int fd) {
    char line[128], buf[128];
    while (fgets(line, sizeof line, stdin) != NULL) {
        size_t len = strlen(line); /* Text input only. Newline sent; NUL not sent. */
        if (write_all(fd, line, len) == -1) die("write");
        size_t got = 0;
        /* Echo protocol: expect exactly as many bytes as this request. */
        while (got < len) {
            ssize_t n = read(fd, buf, len - got);
            if (n > 0) {
                if (write_all(STDOUT_FILENO, buf, (size_t)n) == -1) die("stdout");
                got += (size_t)n;
            } else if (n == 0) {
                fputs("unexpected EOF before complete echo\n", stderr); exit(1);
            } else if (errno != EINTR) die("read");
        }
    }
    if (ferror(stdin)) { fputs("stdin error\n", stderr); exit(1); }
}

static void reap_children(int signal_number) {
    (void)signal_number;
    int saved = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
    errno = saved;
}

int main(int argc, char **argv) {
    /* Convert a broken peer's write failure into an error return. */
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) die("signal");
    setvbuf(stdout, NULL, _IONBF, 0);
    if ((argc == 3 || argc == 4) && strcmp(argv[1], "server") == 0) {
        const char *mode = argc == 4 ? argv[3] : "serial";
        if (strcmp(mode, "serial") && strcmp(mode, "fork-wait") && strcmp(mode, "fork")) {
            fputs("mode must be serial, fork-wait, or fork\n", stderr); return 1;
        }
        if (strcmp(mode, "fork") == 0) {
            struct sigaction action = {0};
            action.sa_handler = reap_children;
            action.sa_flags = SA_RESTART;
            sigemptyset(&action.sa_mask);
            if (sigaction(SIGCHLD, &action, NULL) == -1) die("sigaction");
        }
        /* Local learning demo. Wildcard binding is explained in the notes. */
        int listen_fd = open_socket("127.0.0.1", argv[2], 1);
        if (listen(listen_fd, 16) == -1) die("listen");
        printf("listening on 127.0.0.1:%s\n", argv[2]);
        for (;;) {
            int conn_fd = accept(listen_fd, NULL, NULL);
            if (conn_fd == -1) { if (errno == EINTR) continue; die("accept"); }
            puts("accepted client");
            if (strcmp(mode, "serial") == 0) {
                serve_client(conn_fd);
                close(conn_fd);
                continue;
            }
            pid_t pid = fork();
            if (pid == -1) { perror("fork"); close(conn_fd); continue; }
            if (pid == 0) {
                close(listen_fd);
                serve_client(conn_fd);
                close(conn_fd);
                _exit(0);
            }
            close(conn_fd);
            if (strcmp(mode, "fork-wait") == 0) {
                while (waitpid(pid, NULL, 0) == -1) {
                    if (errno != EINTR) die("waitpid");
                }
            }
        }
    } else if (argc == 4 && strcmp(argv[1], "client") == 0) {
        int fd = open_socket(argv[2], argv[3], 0);
        run_client(fd);
        close(fd);
    } else {
        fprintf(stderr, "usage: %s server PORT [serial|fork-wait|fork] | client HOST PORT\n", argv[0]);
        return 1;
    }
    return 0;
}
