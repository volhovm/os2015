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
int pids[MAX_CONNECTIONS], sockets[MAX_CONNECTIONS];
char *USAGE = "Usage: ./filesender port filename";

void sig_handler(int signo) {
    printf("In handler\n");
    if (signo == SIGCHLD) {
        int i, status, res;
        for (i = 0; i < MAX_CONNECTIONS; i++) {
            if (pids[i] != -1) {
                // try wait
                res = waitpid(pids[i], &status, WNOHANG);
                //printf("waitpid for pid %d returned %d\n", pids[i], res);
                if (res != pids[i]) continue;
                // if succeeded, close socket
                res = close(sockets[i]);
                if (res == -1) printf("Couldn't close socket for child %d\n", pids[i]);
                printf("Ended waiting for pid %d\n", pids[i]);
                pids[i] = -1;
                sockets[i] = -1;
                status = 0;
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
    int s, sfd, pid;
    setbuf(stdout, NULL);
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC; // any version of IP will be OK
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;
    s = getaddrinfo(NULL, argv[1], &hints, &result);
    if (s != 0) {
        printf("Getaddrinfo failed: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1) continue;
        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == -1) {
            close(sfd);
            continue;
        }
        break;
        // binded
    }
    if (rp == NULL) {
        printf("Could not bind to %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(result); // not needed anymore
    SAFERET(listen(sfd, MAX_CONNECTIONS)); // listen accepting connections

    // prevent zombie creation
    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = &sig_handler;
    sigaction(SIGCHLD, &sa, NULL);

    printf("Starting accepting clients\n");
    memset(pids, -1, MAX_CONNECTIONS * sizeof(int));
    memset(sockets, -1, MAX_CONNECTIONS * sizeof(int));
    while (1) {
        struct sockaddr address;
        socklen_t address_len;
        int cfd, i, status;

        // accept new socket
        cfd = accept(sfd, &address, &address_len);
        if (cfd == -1) {
            if (errno != EINTR) printf("error while accepting: %s\n", strerror(errno));
            continue;
        }
        printf("accepted child, sending him something\n");

        // send file in child, track child in parent
        pid = fork();
        if (pid == -1) {
            perror("Could not fork\n");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            sleep(3);
            struct buf_t* buf = buf_new(4096);
            int got, fd, prevsize, err = 0;
            fd = open(argv[2], O_RDONLY);
            if (fd == -1) {
                printf("Could not open %s, child exiting\n", argv[2]);
            }
            while (!err) {
                prevsize = buf_size(buf);
                got = buf_fill(fd, buf, 1);
                if (got == -1) {
                    err = 1;
                    printf("Couldn't fill buffer from child, exiting");
                    break;
                }
                // eof
                if (prevsize == got) {
                    while (buf_size(buf) != 0) buf_flush(fd, buf, 1);
                    break;
                }
                got = buf_flush(cfd, buf, 1);
                if (got == -1) {
                    err = 1;
                    printf("Couldn't flish buffer from child, exiting");
                    break;
                }
            }
            buf_free(buf);
            close(fd);
            //shutdown(cfd, SHUT_RDWR);
            close(cfd);
            if (err) {
                exit(EXIT_FAILURE);
            } else {
                printf("ended sending file to fd %d\n", cfd);
                exit(EXIT_SUCCESS);
            }
        } else {
            for (i = 0; i < MAX_CONNECTIONS; i++) {
                if (pids[i] == -1 && sockets[i] == -1) {
                    pids[i] = pid;
                    sockets[i] = cfd;
                    printf("succesfully forked, sending %s to fd %d from pid %d\n",
                           argv[2], cfd, pid);
                    break;
                }
            }
        }
    }
    return 1;
}
