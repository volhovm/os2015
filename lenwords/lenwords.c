#include <helpers.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int BUFFER_SIZE = 4096;

void main() {
    char buf[BUFFER_SIZE];
    int margin = 0;
    while (1) {
        int got = read_until(STDIN_FILENO, buf + margin, BUFFER_SIZE / 4, ' ');
        if (got == 0) {
            char length[4096];
            int out = sprintf(length, "%d", margin);
            write_(STDOUT_FILENO, length, out);
            break;
        }
        int i;
        int last_mark = 0;
        for (i = margin; i < margin + got; i++) {
            if (buf[i] == ' ') {
                int tmpsize = i - last_mark;
                char length[4096];
                int out = sprintf(length, "%d ", tmpsize);
                write_(STDOUT_FILENO, length, out);
                fflush(stdout);
                last_mark = i + 1;
            }
        }
        memcpy(buf, buf + last_mark, got + margin - last_mark + 1);
        margin = got + margin - last_mark;
    }
}
