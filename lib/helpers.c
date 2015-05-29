#include <helpers.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/wait.h>

#define SAFERET(a) if (a == -1) return -1;

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
        w = waitpid(cpid, &status, WUNTRACED | WCONTINUED);
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

int exec(struct execargs_t* args) {
    return spawn(args->name, args->args);
}

// Removes all pending signals and unblock the mask
// returns -1 if EINTR
int signals_unblock(sigset_t* smask) {
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
    //printf("\nstate: %d %d\n", n, write_to);
    if (n == 1) {
        int res = dup2(write_to, STDOUT_FILENO);
        SAFERET(res);
        exec(programs[0]);
        return 0;
    }
    int pid;
    int pipefd[2];
    int res = pipe(pipefd);
    SAFERET(res);
    pid = fork();
    if (pid == -1) {
        perror("fork");
        return -1;
    }
    if (pid == 0) {
        close(pipefd[0]);
        //close(STDERR_FILENO);
        return runpiped1(programs, n-1, pipefd[1], smask);
    } else {
        printf("Forked: %d", pid);
        fflush(stdout);
        close(pipefd[1]);

        int stdout_backup = dup(STDOUT_FILENO);
        SAFERET(stdout_backup);
        int stdin_backup = dup(STDIN_FILENO);
        SAFERET(stdin_backup);

        SAFERET(dup2(write_to, STDOUT_FILENO));
        SAFERET(dup2(pipefd[0], STDIN_FILENO));
        exec(programs[n - 1]);
        SAFERET(dup2(stdin_backup, STDIN_FILENO));
        SAFERET(dup2(stdout_backup, STDOUT_FILENO));

        siginfo_t siginfo;
        int sig = sigwaitinfo(smask, &siginfo);
        SAFERET(sig);

        int status;
        close(pipefd[0]);
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}

int runpiped(struct execargs_t** programs, size_t n) {
    sigset_t smask;
    sigemptyset(&smask);
    sigaddset(&smask, SIGINT);
    sigaddset(&smask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &smask, NULL);
    int res = runpiped1(programs, n, STDOUT_FILENO, &smask);
    sigprocmask(SIG_UNBLOCK, &smask, NULL);
    return res;
}
