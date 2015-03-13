#include "helpers.h"
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

ssize_t read_(int fd, void *buf, size_t count) {
    int got_chars = 0;
    int delimiter_found = 0;
    while (count > got_chars && !delimiter_found) {
        int got = read(fd, buf, count);
        if (got == 0) break;
        int i;
        for (i = 0; i < got; i++) {
            if (((char*) buf)[i] == -1) {
                delimiter_found = 1;
                break;
            }
            got_chars++;
        }
        buf += got;
    }
    return got_chars;
}

ssize_t write_(int fd, const void *buf, size_t count) {
    int got_chars = 0;
    int delimiter_found = 0;
    while (count > got_chars && !delimiter_found) {
        int got = write(fd, buf, count);
        if (got == 0) break;
        int i;
        for (i = 0; i < got; i++) {
            if (((char*) buf)[i] == -1) {
                delimiter_found = 1;
                break;
            }
            got_chars++;
        }
        buf += got;
    }
    return got_chars;
}
