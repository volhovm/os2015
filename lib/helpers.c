#include <helpers.h>
#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/wait.h>

#define SAFERET(a) if (a == -1) { printf("SUPER ERROR"); return -1; }

ssize_t read_until(int fd, void* buf, size_t count, char delimiter) {
    int got_chars = 0;
    int delimiter_found = 0;
    while (count > got_chars && !delimiter_found) {
        int got = read(fd, buf, count);
        if (got == 0) break;
        int i;
        got_chars += got;
        for (i = 0; i < got; i++) {
            if (((char*) buf)[i] == delimiter) {
                delimiter_found = 1;
                break;
            }
        }
        buf += got;
    }
    return got_chars;
}

ssize_t read_(int fd, void* buf, size_t count) {
    int got_overall = 0;
    while (1) {
        printf("TEST");
        int got = read(fd, buf, count);
        printf("TEST_AFTER");
        if (got == -1) return -1;
        if (got == 0) break;
        fflush(stdout);
        got_overall += got;
        buf += got;
        count -= got;
    }
    printf("READ_ EXIT");
    fflush(stdout);

    return got_overall;
}

ssize_t write_(int fd, const void* buf, size_t count) {
    int got_chars = 0;
    while (count > got_chars) {
        int got = write(fd, buf, count);
        if (got == 0) break;
        int i;
        got_chars += got;
        for (i = 0; i < got; i++) {
            if (((char*) buf)[i] == -1) {
                break;
            }
        }
        buf += got;
    }
    return got_chars;
}

int spawn(const char* file, char* const argv []) {
    int w, status;
    int cpid = fork();
    if (cpid == -1) {
        perror("fork");
        return -1;
    }
    if (cpid == 0) {
        if (execvp(file, argv) == -1) {
            perror("execvp");
            return -1;
        }
    } else {
        w = waitpid(cpid, &status, 0);
        SAFERET(w);
        if (!WIFEXITED(status) | WIFSIGNALED(status)) return -1;
        return WEXITSTATUS(status);
    }
    return 0;
}



struct execargs_t* execargs_new(char* name, char* args[]) {
    void* space = malloc(sizeof (struct execargs_t));
    if (space == NULL) return NULL;
    struct execargs_t* ret = (struct execargs_t*) space;
    ret->args = args;
    ret->name = name;
    return ret;
}

struct execargs_t* execargs_fromargs(char** args) {
    return execargs_new(args[0], args);
}

sigset_t intchldmask() {
    sigset_t ret;
    sigemptyset(&ret);
    sigaddset(&ret, SIGINT);
    sigaddset(&ret, SIGCHLD);
    return ret;
}

// executes a given program.
// returns exit status of program
int exec(struct execargs_t* prog) {
    return spawn(prog->name, prog->args);
    //    int w, status;
    //    int cpid = fork();
    //    if (cpid == -1) {
    //        perror("fork");
    //        return -1;
    //    }
    //    if (cpid == 0) {
    //        if (execvp(args->name, args->args) == -1) {
    //            perror("execvp");
    //            return -1;
    //        }
    //    } else {
    //        w = waitpid(cpid, &status, 0);
    //        SAFERET(w);
    //        if (!WIFEXITED(status) | WIFSIGNALED(status)) return -WTERMSIG(status)-1;
    //        return WEXITSTATUS(status);
    //    }
    //    return 0;
}

// Removes signal id if any signal is pending, 0 otherwise, -1 if error
int signals_first(const sigset_t* smask) {
    struct timespec timeout;
    timeout.tv_sec=0;
    timeout.tv_nsec=10;
    siginfo_t info;
    int res = sigtimedwait(smask, &info, &timeout);
    if (res == -1 && errno == EAGAIN) return 0;
    return res;
}

// Removes all pending signals and unblock the mask
// returns -1 if EINTR
int signals_unblock(const sigset_t* smask) {
    struct timespec timeout;
    timeout.tv_sec=0;
    timeout.tv_nsec=10;
    siginfo_t info;
    while (1) {
        int res = sigtimedwait(smask, &info, &timeout);
        if (res == -1) {
            if (errno == EAGAIN) break;
            sigprocmask(SIG_UNBLOCK, smask, NULL);
            return -1;
        }
    }

    sigprocmask(SIG_UNBLOCK, smask, NULL);
    return 0;
}

int runpiped1(struct execargs_t** programs, size_t n, int write_to, sigset_t* smask) {
    assert(n > 0);
    int pid, sig, res, status;
    int pipefd[2];
    res = pipe(pipefd);
    SAFERET(res);
    pid = fork(); // child executes n-1 program, parent is running n'th
    if (pid == -1) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        close(pipefd[0]);
        if (n == 1) {
            SAFERET(close(pipefd[1]));
            exit(0);
        } else {
            exit(runpiped1(programs, n - 1, pipefd[1], smask));
        }
    } else {
        siginfo_t siginfo;
        int childkilled = 0;
        int retcode = 0;
        printf("depth %zu:: Forked: %d\n", n, pid);
        close(pipefd[1]);

        // process all signals that could be generated up to this point
        // if SIGINT, kill child, else execute the given program
        sig = signals_first(smask);
        SAFERET(sig);
        if (sig == SIGTERM) {
            kill(pid, SIGINT);
        } else {
            if (sig == SIGCHLD) childkilled = 1;

            // start program #n-1
            int cpid = fork();
            SAFERET(cpid);
            if (cpid == 0) {
                // let execed child response to SIGINT and SIGCHLD (work as it should)
                signals_unblock(smask);
                // dup2 new descriptors to stdout, stdin
                SAFERET(dup2(write_to, STDOUT_FILENO));
                if (n > 1) SAFERET(dup2(pipefd[0], STDIN_FILENO));
                // close init fds
                if (n > 1) SAFERET(close(write_to));
                SAFERET(close(pipefd[0]));
                if (execvp(programs[n-1]->name, programs[n-1]->args) == -1) {
                    perror("execvp");
                    return -1;
                }
            } else {
                int execed_dead = 0;
                int newpid;
                printf("depth %zu:: exec'ed %d\n", n, cpid);
                // sent SIGINT to child if needed and wait for its execution end
                while (!execed_dead) {
                    printf("depth %zu:: waiting for a signal after exec\n", n);
                    sig = sigwaitinfo(smask, &siginfo);
                    SAFERET(sig);
                    if (sig == SIGINT) {
                        printf("depth %zu:: got sigint, relaying it to exec-chld (%d)\n", n, cpid);
                        SAFERET(kill(cpid, SIGINT));
                        if (!childkilled) {
                            //printf("depth %zu:: and to direct child %d\n", n, pid);
                            SAFERET(kill(pid, SIGINT));
                        }
                    } else if (sig == SIGCHLD) {
                        // wait all closed childs (max 2) to prevent zombie creation
                        while ( (newpid = waitpid(-1, &status, WNOHANG)) != -1
                                && errno != ECHILD ) {
                            if (newpid == -1) {
                                perror("wait");
                                return -1;
                            }
                            if (newpid == pid) {
                                retcode = siginfo.si_status;
                                childkilled = 1;
                                printf("depth %zu:: previous-chain child #%d died\n", n, pid);
                            } else if (newpid == cpid) {
                                SAFERET(close(pipefd[0]));
                                execed_dead = 1;
                                printf("depth %zu:: exec'ed child #%d has finished\n", n, cpid);
                            }
                        }
                    }
                }
            }
            // the exec'ed program is done by here, and we have to wait for SIGINT
            // or child death (if it hasn't happened due to this point).
        }

        printf("depth %zu:: before last while\n", n);
        while (!childkilled) {
            sig = sigwaitinfo(smask, &siginfo);
            SAFERET(sig);
            printf("depth %zu:: got signal %d in last while\n", n, sig);
            // if got SIGCHLD, wait pid, save return code and return
            // if got signal from parent (SIGINT), propagate to child and wait for SIGCHLD
            if (sig == SIGINT) {
                // ignored, each process child gets it's SIGTERM automatically
                //SAFERET(kill(pid, SIGINT));
            } else if (sig == SIGCHLD) {
                SAFERET(waitpid(pid, &status, WNOHANG));
                retcode = siginfo.si_status;
                childkilled = 1;
            }
        }
        printf("depth %zu:: exiting\n", n);
        return retcode;
    }
}

int runpiped(struct execargs_t** programs, size_t n) {
    // create a mask that contains {SIGINT, SIGCHLD} \ {current mask}
    sigset_t smask, sdelmask, sinitmask;
    smask = intchldmask();
    sigemptyset(&sdelmask);
    sigprocmask(SIG_SETMASK, NULL, &sinitmask);
    if (!sigismember(&sinitmask, SIGINT)) {
        sigaddset(&sdelmask, SIGINT);
    }
    if (!sigismember(&sinitmask, SIGCHLD)) {
        sigaddset(&sdelmask, SIGCHLD);
    }
    sigprocmask(SIG_BLOCK, &sdelmask, NULL);

    // start programs in child, wait for them in parent
    int pid = fork();
    if (pid == -1) return -1;
    if (pid == 0) {
        int res = runpiped1(programs, n, STDOUT_FILENO, &smask);
        printf("master-child: %d\n", res);
        signals_unblock(&sdelmask);
        exit(res);
    } else {
        // wait for any signal
        siginfo_t siginfo;
        int sig, status, retvalue = 0;

        printf("master:: Forked init: %d\n", pid);

        // kill child in case of SIGINT, in case of SIGCHLD exit normally
        while (1) {
            sig = sigwaitinfo(&smask, &siginfo);
            printf("master:: sig is %d\n", sig);
            SAFERET(sig);

            if (sig == SIGINT) {
                SAFERET(kill(pid, SIGINT));
            } else if (sig == SIGCHLD) {
                SAFERET(waitpid(pid, &status, WNOHANG));
                retvalue = WEXITSTATUS(status);
                break;
            }
        }
        signals_unblock(&sdelmask);
        printf("exited from master\n");
        return retvalue;
    }
}
