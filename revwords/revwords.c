#include "../lib/helpers.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void main() {
    char buf[4096];
    int already_read = 0;
    while (1) {
        int got = read_until(STDIN_FILENO, buf, 4096, ' ');
        if (got == 0) break;
        // ??
    }
}
