#include "../lib/helpers.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

void main() {
    char buf[8200];
    int margin = 0;
    while (1) {
        int got = read_until(STDIN_FILENO, buf + margin * sizeof (char), 4096, ' ');
        //        write(STDOUT_FILENO, buf + margin * sizeof (char), got);
        if (got == 0) {
            int j;
            for (j = 0; j < margin / 2; j++) {
                char temp = buf[margin - j - 1];
                buf[margin - j - 1] = buf[j];
                buf[j] = temp;
            }
            write(STDOUT_FILENO, buf, margin);
            break;
        }
        printf("got: %d, margin: %d \n", got, margin);
        int i;
        int last_mark = 0;
        for (i = margin; i < margin + got; i++) {
            if (buf[i] == ' ') {
                int tmpsize = i - last_mark;
                char currstr[tmpsize];
                memcpy(currstr, buf + last_mark, tmpsize);
                int j;
                for (j = 0; j < tmpsize / 2; j++) {
                    char temp = currstr[tmpsize - j - 1];
                    currstr[tmpsize - j - 1] = currstr[j];
                    currstr[j] = temp;
                }
                write(STDOUT_FILENO, currstr, tmpsize);
                write(STDOUT_FILENO, " ", 1);
                fflush(stdout);
                last_mark = i + 1;
            }
        }
        memcpy(buf, buf + last_mark * sizeof (char), got + margin - last_mark + 1);
        margin = got + margin - last_mark;
    }
}
