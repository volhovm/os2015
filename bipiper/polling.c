#include <bufio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <wait.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
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

int ONE_WAY_CONNECTION_ALLOWED = 1;
int MAX_SOCKETS = 128;
int BUFFER_SIZE = 4096;
int VERBOSE = 1;
char *USAGE = "Usage: ./polling port1 port2";

struct bufpair {
    struct buf_t *b1, *b2;
};

void logm(int fd, const char *format, ...) {
    if (fd == STDERR_FILENO || (fd == STDOUT_FILENO && VERBOSE)) {
        va_list args;
        va_start(args, format);
        if (fd == STDERR_FILENO) vfprintf(stderr, format, args);
        else vfprintf(stdout, format, args);
        va_end(args);
    }
}

void dump_mask(short mask) {
    printf("mask: %d, POLLIN: %d, POLLOUT: %d, POLLERR: %d, POLLHUP: %d, POLLNVAL: %d\n",
           mask,
           mask & POLLIN,
           mask & POLLOUT,
           mask & POLLERR,
           mask & POLLHUP,
           mask & POLLNVAL);
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
    SAFERET(listen(*sfd, MAX_SOCKETS - 1)); // listen accepting connections on port1
    return 0;
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

    // not needed anymore
    freeaddrinfo(result);

    logm(STDOUT_FILENO, "Starting accepting clients\n");

    struct pollfd pollfds[2 * MAX_SOCKETS];
    memset(pollfds, 0, 2 * MAX_SOCKETS * sizeof(struct pollfd));
    struct bufpair bufs[MAX_SOCKETS]; // in each pair, b1 is from 2n to 2n+1, b2 is conversely
    memset(bufs, 0, MAX_SOCKETS * sizeof(struct bufpair));
    int valid_in[MAX_SOCKETS];
    int valid_out[MAX_SOCKETS];
    memset(valid_in, 0, 2 * MAX_SOCKETS * sizeof(int));
    memset(valid_out, 0, 2 * MAX_SOCKETS * sizeof(int));

    size_t size = 1;
    pollfds[0].fd = sfd1;
    pollfds[0].events = POLLIN;
    pollfds[1].fd = sfd2;
    valid_in[0] = valid_out[0] = 1;
    valid_in[1] = valid_out[1] = 1;

    for (;;) {
        struct sockaddr address;
        socklen_t address_len;
        int cfd1, cfd2, i, num, snd, twin;
        struct buf_t *fbuf, *bbuf; // buffer from you to twin, and backwards

        logm(STDOUT_FILENO, "\nBefore poll\n");
        num = poll(pollfds, size * 2, -1);
        //logm(STDOUT_FILENO, "Got from poll: %d\n", num);
        for (i = 0; i < 2 * size; i++) {
            if (pollfds[i].revents != 0) {
                snd = i % 2;
                if (snd) {
                    twin = i - 1;
                    fbuf = bufs[i / 2].b2;
                    bbuf = bufs[i / 2].b1;
                } else {
                    twin = i + 1;
                    fbuf = bufs[i / 2].b1;
                    bbuf = bufs[i / 2].b2;
                }

                // help values
                switch (i) {
                case 0:
                    if (pollfds[i].revents & POLLIN) {
                        pollfds[0].events = 0;
                        pollfds[1].events = POLLIN;
                        cfd1 = accept(sfd1, &address, &address_len);
                        logm(STDOUT_FILENO, "Successfully accepted socket1\n");
                    } else {
                        logm(STDERR_FILENO, "Error occured in poll/1\n");
                        dump_mask(pollfds[0].revents);
                        exit(0);
                    }
                    break;
                case 1:
                    if (pollfds[i].revents & POLLIN) {
                        pollfds[0].events = POLLIN;
                        pollfds[1].events = 0;
                        cfd2 = accept(sfd2, &address, &address_len);
                        logm(STDOUT_FILENO, "Successfully accepted socket2\n");

                        // add new pair of sockets, subscribe to both writes and reads
                        pollfds[2*size].fd = cfd1;
                        pollfds[2*size].events = POLLIN | POLLOUT;
                        pollfds[2*size+1].fd = cfd2;
                        pollfds[2*size+1].events = POLLIN | POLLOUT;
                        // set up buffers for them
                        bufs[size].b1 = buf_new(BUFFER_SIZE);
                        bufs[size].b2 = buf_new(BUFFER_SIZE);
                        // mark them as capable to read/write
                        valid_in[2*size] = valid_out[2*size] = 1;
                        valid_in[2*size+1] = valid_out[2*size+1] = 1;
                        size++;
                    } else {
                        logm(STDERR_FILENO, "Error occured in poll/2\n");
                        dump_mask(pollfds[1].revents);
                        exit(0);
                    }
                    break;
                default:
                    //logm(STDOUT_FILENO, "Something happend on %d\n", i);
                    //dump_mask(pollfds[i].revents);
                    // POLLIN
                    if (pollfds[i].revents & POLLIN) {
                        logm(STDOUT_FILENO, "Pollin on %d\n", i);
                        // mark this socket as invalid if other part is invalid
                        // because there's no purpose in reading without intention to write
                        if (!valid_out[twin]) {
                            valid_in[i] = 0;
                            pollfds[i].events &= ~POLLIN;
                            shutdown(pollfds[i].fd, SHUT_RD);
                            logm(STDOUT_FILENO, "Twin is not valid, ~POLLIN, "
                                 "unvaliding, shutdown\n");
                            break;
                        }

                        // try to fill buffer
                        // if buffer is full, unsubscribe from reading (to prevent burning cpu)
                        if (buf_size(fbuf) == buf_capacity(fbuf)) {
                            pollfds[i].events &= ~POLLIN;
                            logm(STDOUT_FILENO, "Buffer is full, ~POLLIN\n");
                            break;
                        } else s = buf_fill(pollfds[i].fd, fbuf, 1);

                        // tried to fill and got error or eof ⇒
                        // can't read from it, unsubscribe from reads, mark as invalid
                        if (s < 1) {
                            pollfds[i].events &= ~POLLIN;
                            valid_in[i] = 0;
                            shutdown(pollfds[i].fd, SHUT_RD);
                            logm(STDOUT_FILENO, "Failed to fill buffer, ~POLLING, unvaliding\n");
                        }

                        // subscribe read side to reading, as buffer is not empty now
                        // or just say twin that you're done
                        if (valid_out[twin] && s >= 0) {
                            pollfds[twin].events |= POLLOUT;
                        }
                    }
                    // POLLOUT
                    if (pollfds[i].revents & POLLOUT) {
                        logm(STDOUT_FILENO, "Pollout on %d\n", i);
                        // mark this as invalid, if write side is invalid
                        // and back buffer (to us) is empty
                        if (!valid_in[twin] && buf_size(bbuf) == 0) {
                            valid_out[i] = 0;
                            pollfds[i].events &= ~POLLOUT;
                            shutdown(pollfds[i].fd, SHUT_WR);
                            logm(STDOUT_FILENO, "Twin is not valid and buffer is empty "
                                                "~POLLOUT, unvaliding, shutdown\n");
                            break;
                        }

                        // the same for valid start, just unsubscribe
                        if (buf_size(bbuf) == 0) {
                            pollfds[i].events &= ~POLLOUT;
                            logm(STDOUT_FILENO, "Buffer is empty, ~POLLOUT\n");
                            break;
                        }

                        // flush buffer
                        s = buf_flush(pollfds[i].fd, bbuf, 1);

                        // if failed, mark as invalid and unsubscribe, and shutdown
                        if (s < 1) {
                            valid_out[i] = 0;
                            pollfds[i].events &= ~POLLOUT;
                            shutdown(pollfds[i].fd, SHUT_WR);
                            logm(STDOUT_FILENO, "Failed to flush the buffer (s=%d), "
                                 "~POLLOUT, unvaliding, shutdown\n", s);
                            break;
                        }

                        // if write succeeded, and pipe start is valid,
                        // it could be unsubscribed from POLLIN because of filled buffer,
                        // so subscribe it to POLLIN
                        if (valid_in[twin]) {
                            pollfds[twin].events |= POLLIN;
                        }
                    }
                    if ((pollfds[i].revents & POLLERR) || (pollfds[i].revents & POLLHUP)) {
                        logm(STDOUT_FILENO,
                             "Got POLERR/POLLHUP, "
                             "disconnecting socket, reporting to twin\n");
                        pollfds[i].events = 0; // unsubscribe from everything
                        if (valid_in[i]) {
                            valid_in[i] = 0;
                            // report another socket
                            if (valid_out[twin]) {
                                pollfds[twin].events |= POLLOUT;
                            }
                        }
                        if (valid_out[i]) {
                            valid_out[i] = 0;
                            if (valid_in[twin]) {
                                pollfds[twin].events |= POLLIN;
                            }
                        }
                    }
                    break;
                }
            }
        }
        for (i = 0; i < size; i++) {

            if (// if ONE_WAY_CONNECTION_ALLOWED, then disconnect if all 4 are invalid
                !valid_in[2*i] && !valid_in[2*i+1] && !valid_out[2*i] && !valid_out[2*i+1]
                && ONE_WAY_CONNECTION_ALLOWED ||
                // else check if at least one pair is invalid
                (!valid_in[2*i] && !valid_in[2*i+1]) || (!valid_out[2*i] && !valid_out[2*i+1])
                ) {
                logm(STDOUT_FILENO, "Closing pair %d\n", i);
                // close everything that needs to be closed
                close(pollfds[2*i].fd);
                close(pollfds[2*i+1].fd);
                buf_free(bufs[i].b1);
                buf_free(bufs[i].b2);

                // swap with last elements if there are more than one pair of connections
                if (size > 2) {
                    valid_in[2*i] = valid_in[size*2];
                    valid_in[2*i+1] = valid_in[size*2+1];
                    pollfds[2*i] = pollfds[size*2];
                    pollfds[2*i+1] = pollfds[size*2+1];
                    bufs[i] = bufs[size];
                }
                size--;
                logm(STDOUT_FILENO, "Closed pair %d\n", i);
                i--; // not miss the final swapped element
            }
        }

    }
    close(sfd1);
    close(sfd2);
    return 1;
}
