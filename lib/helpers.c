#include <helpers.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>

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
        if (w == -1) return -1;
        if (!WIFEXITED(status) | WIFSIGNALED(status)) return -1;
        return WEXITSTATUS(status);
    }
    return 0;
}

struct execargs_t* execargs_new(char* name, char** args) {
    return NULL;
}

int exec(struct execargs_t* args) {
    return 1;
}

int runpiped(struct execargs_t** programs, size_t n) {
    return 1;
}
