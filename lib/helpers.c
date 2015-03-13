#include "helpers.h"
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>

ssize_t read_(int fd, void *buf, size_t count) {
    int pack_size = 30;
    int delimiter_found = 0;
    while (count > 0 && !delimiter_found) {
        int got = read(fd, buf, pack_size);
        count -= got;
        int i;
        for (i = 0; i < got; i++) {
            if (((char*) buf)[i] == -1) {
                delimiter_found = 1;
                break;
            }
            count--;
        }
        buf += got;
    }
    return count;
}

ssize_t write_(int fd, const void *buf, size_t count) {
    int pack_size = 30;
    int delimiter_found = 0;
    while (count > 0 && !delimiter_found) {
        int got = write(fd, buf, pack_size);
        count -= got;
        int i;
        for (i = 0; i < got; i++) {
            if (((char*) buf)[i] == -1) {
                delimiter_found = 1;
                break;
            }
            count--;
        }
        buf += got;
    }
    return count;
}
