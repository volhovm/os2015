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

int MAX_CONNECTIONS = 500;
char *USAGE = "Usage: ./filesender port filename";

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

    printf("Starting accepting clients\n");
    int pids[MAX_CONNECTIONS];
    int sfds[MAX_CONNECTIONS];
    memset(pids, -1, MAX_CONNECTIONS * sizeof(int));
    memset(sfds, -1, MAX_CONNECTIONS * sizeof(int));
    for (;;) {
        struct sockaddr address;
        socklen_t address_len;
        int cfd, i, status;

        // quickly poll all pids if they are finished
        for (i = 0; i < MAX_CONNECTIONS; i++) {
            if (pids[i] != -1) {
                s = waitpid(pids[i], &status, WNOHANG);
                printf("waited for pid %d\n", pids[i]);
                if (s == -1) {
                    printf("waitpid for pid %d failed\n", pids[i]);
                    exit(EXIT_FAILURE);
                }
                close(sfds[i]);
                sfds[i] = -1;
                pids[i] = -1;
            }
        }

        // accept new socket
        cfd = accept(sfd, &address, &address_len);
        printf("accepted child, sending him something\n");
        if (cfd == -1) {
            printf("error while accepting: %s\n", strerror(errno));
            continue;
        }

        // send file in child, track child in parent
        pid = fork();
        if (pid == -1) {
            perror("Could not fork\n");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            struct buf_t* buf = buf_new(4096);
            int got, fd, prevsize;
            fd = open(argv[2], O_RDONLY);
            if (fd == -1) {
                printf("Could not open %s, child exiting\n", argv[2]);
                exit(EXIT_FAILURE);
                close(cfd);
                buf_free(buf);
            }
            for (;;) {
                prevsize = buf_size(buf);
                got = buf_fill(fd, buf, 1);
                if (got == -1) {
                    buf_free(buf);
                    close(fd);
                    close(cfd);
                    printf("Couldn't fill buffer from child, exiting");
                    exit(EXIT_FAILURE);
                }
                // eof
                if (prevsize == got) {
                    while (buf_size(buf) != 0) buf_flush(fd, buf, 1);
                    break;
                }
                got = buf_flush(cfd, buf, 1);
                if (got == -1) {
                    buf_free(buf);
                    close(fd);
                    close(cfd);
                    printf("Couldn't fill buffer from child, exiting");
                    exit(EXIT_FAILURE);
                }
            }
            buf_free(buf);
            close(fd);
            close(cfd);
            printf("ended sending file to fd %d\n", cfd);
            exit(EXIT_SUCCESS);
        }
        // put pid into array (first empty place)
        for (i = 0; i < MAX_CONNECTIONS; i++) {
            if (pids[i] == -1) {
                pids[i] = pid;
                sfds[i] = cfd;
                break;
            }
        }
        printf("succesfully forked, sending %s to fd %d from pid %d\n", argv[2], cfd, pid);
    }
    return 1;
}
