#include <helpers.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

const int BUFFER_SIZE = 4096;

int main() {
    char temp_buf[BUFFER_SIZE];
    while (1) {
        int got = read_(STDIN_FILENO, temp_buf, BUFFER_SIZE / 2);
        if (got < 0) return -1;
        if (got == 0) break;
        int pushed = write_(STDOUT_FILENO, temp_buf, got);
        if (pushed == -1) break;
        fflush(stdout);
    }
    return 0;
}
