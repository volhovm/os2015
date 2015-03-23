#include "../lib/helpers.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    char temp_buf[1000];
    while (1) {
        int got = read_(STDIN_FILENO, temp_buf, 1000 * sizeof(char));
        if (got < 0) return -1;
        if (got == 0) break;
        int pushed = write_(STDOUT_FILENO, temp_buf, got);
    }
    return 0;
}
