#include <bufio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <wait.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>

#define DEBUG

#ifdef DEBUG
void assume(int res) { if (res != 0) abort(); }
#else
void assume(int res) { }
#endif

#define SAFERET(a) { if (a != 0) return -1; }

#define MAX_CONNECTIONS 500

int VERBOSE = 1;
char *USAGE = "Usage: ./forking port1 port2";

// logs info
void logm(int fd, const char *format, ...) {
    if (fd == STDERR_FILENO || (fd == STDOUT_FILENO && VERBOSE)) {
        va_list args;
        va_start(args, format);
        if (fd == STDERR_FILENO) vfprintf(stderr, format, args);
        else vfprintf(stdout, format, args);
        va_end(args);
    }
}

// initializes server sockets {bind; listen}
int init_sfd(char *port, int *sfd, struct addrinfo **result, struct addrinfo *hints) {
    int s, one = 1;
    struct addrinfo *rp;
    s = getaddrinfo(NULL, port, hints, result);
    if (s != 0) {
        logm(STDERR_FILENO, "Getaddrinfo failed: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }
    for (rp = *result; rp != NULL; rp = rp->ai_next) {
        *sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (*sfd == -1) continue;
        if (setsockopt(*sfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1)
            perror("setsockopt");
        if (bind(*sfd, rp->ai_addr, rp->ai_addrlen) == -1) {
            close(*sfd);
            continue;
        }
        break;
        // binded
    }
    if (rp == NULL) {
        logm(STDERR_FILENO, "Could not bind to %s : %s\n", port, gai_strerror(errno));
        exit(EXIT_FAILURE);
    }
    SAFERET(listen(*sfd, MAX_CONNECTIONS)); // listen accepting connections on port1
    return 0;
}

int pids1[MAX_CONNECTIONS];
int pids2[MAX_CONNECTIONS];
int sfds1[MAX_CONNECTIONS];
int sfds2[MAX_CONNECTIONS];

void sig_handler(int signum) {
    if (signum == SIGCHLD) {
        int i, status, s;
        for (i = 0; i < MAX_CONNECTIONS; i++) {
            if (pids1[i] != -1) {
                s = waitpid(pids1[i], &status, WNOHANG);
                printf("waited for pid %d\n", pids1[i]);
                if (s == -1) {
                    printf("waitpid for pid %d failed\n", pids1[i]);
                    exit(EXIT_FAILURE);
                }
                close(sfds1[i]);
                sfds1[i] = -1;
                pids1[i] = -1;
            }
            if (pids2[i] != -1) {
                s = waitpid(pids2[i], &status, WNOHANG);
                printf("waited for pid %d\n", pids2[i]);
                if (s == -1) {
                    printf("waitpid for pid %d failed\n", pids2[i]);
                    exit(EXIT_FAILURE);
                }
                close(sfds2[i]);
                sfds2[i] = -1;
                pids2[i] = -1;
            }
        }

    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("%s\n", USAGE);
        exit(EXIT_FAILURE);
    }
    struct addrinfo hints, *result, *rp;
    int s, sfd1, sfd2;
    setbuf(stdout, NULL);
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC; // any version of IP will be OK
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;

    // bind to ports, listen on them
    SAFERET(init_sfd(argv[1], &sfd1, &result, &hints));
    SAFERET(init_sfd(argv[2], &sfd2, &result, &hints));

    freeaddrinfo(result); // not needed anymore

    struct sigaction sact;
    bzero(&sact, sizeof(sact));
    sact.sa_handler = &sig_handler;
    sigaction(SIGCHLD, &sact, NULL);

    printf("Starting accepting clients\n");
    memset(pids1, -1, MAX_CONNECTIONS * sizeof(int));
    memset(sfds1, -1, MAX_CONNECTIONS * sizeof(int));
    memset(pids2, -1, MAX_CONNECTIONS * sizeof(int));
    memset(sfds2, -1, MAX_CONNECTIONS * sizeof(int));
    for (;;) {
        struct sockaddr address;
        socklen_t address_len;
        int cfd1, cfd2, i, status, pid1, pid2;

        // quickly poll all pids if they are finished

        // accept new socket1
        cfd1 = accept(sfd1, &address, &address_len);
        if (cfd1 == -1) {
            printf("error while accepting: %s\n", strerror(errno));
            continue;
        }
        printf("accepted child1 \n");

        // accept new socket2
        cfd2 = accept(sfd2, &address, &address_len);
        if (cfd2 == -1) {
            if (errno != EINTR) printf("error while accepting: %s\n", strerror(errno));
            continue;
        }
        printf("accepted child2 \n");

        // fork twice, first is reading from cfd1 and sending to cfd2, set up connections
        pid1 = fork();
        if (pid1 == -1) {
            perror("Could not fork\n");
            exit(EXIT_FAILURE);
        }
        if (pid1 == 0) {
            struct buf_t* buf = buf_new(4096);
            int got, prevsize;
            for (;;) {
                prevsize = buf_size(buf);
                got = buf_fill(cfd1, buf, 1);
                if (got == -1) {
                    buf_free(buf);
                    shutdown(cfd1, SHUT_RD);
                    shutdown(cfd2, SHUT_WR);
                    printf("Couldn't fill buffer from child1, exiting");
                    exit(EXIT_FAILURE);
                }
                // eof
                if (prevsize == buf_size(buf)) {
                    while (buf_size(buf) != 0) buf_flush(cfd2, buf, 1);
                    break;
                }
                got = buf_flush(cfd2, buf, 1);
                if (got == -1) {
                    buf_free(buf);
                    shutdown(cfd1, SHUT_RD);
                    shutdown(cfd2, SHUT_WR);
                    printf("Couldn't flush buffer from child1, exiting");
                    exit(EXIT_FAILURE);
                }
            }
            buf_free(buf);
            shutdown(cfd1, SHUT_RD);
            shutdown(cfd2, SHUT_WR);
            exit(EXIT_SUCCESS);
        }

        // fork second time
        pid2 = fork();
        if (pid2 == -1) {
            perror("Could not fork\n");
            exit(EXIT_FAILURE);
        }
        if (pid2 == 0) {
            struct buf_t* buf = buf_new(4096);
            int got, prevsize;
            for (;;) {
                prevsize = buf_size(buf);
                got = buf_fill(cfd2, buf, 1);
                if (got == -1) {
                    buf_free(buf);
                    shutdown(cfd2, SHUT_RD);
                    shutdown(cfd1, SHUT_WR);
                    printf("Couldn't fill buffer from child1, exiting");
                    exit(EXIT_FAILURE);
                }
                // eof
                if (prevsize == buf_size(buf)) {
                    while (buf_size(buf) != 0) buf_flush(cfd1, buf, 1);
                    break;
                }
                got = buf_flush(cfd1, buf, 1);
                if (got == -1) {
                    buf_free(buf);
                    shutdown(cfd2, SHUT_RD);
                    shutdown(cfd1, SHUT_WR);
                    printf("Couldn't flush buffer from child1, exiting");
                    exit(EXIT_FAILURE);
                }

            }
            buf_free(buf);
            shutdown(cfd2, SHUT_RD);
            shutdown(cfd1, SHUT_WR);
            exit(EXIT_SUCCESS);
        }

        // put pid into array (first empty place)
        for (i = 0; i < MAX_CONNECTIONS; i++) {
            if (pids1[i] == -1 && pids2[i] == -1) {
                pids1[i] = pid1;
                sfds1[i] = cfd1;
                pids2[i] = pid2;
                sfds2[i] = cfd2;
                break;
            }
        }
    }
    close(sfd1);
    close(sfd2);
    return 1;
}
