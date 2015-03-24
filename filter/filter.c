#include <helpers.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

void main() {
    char *buf;
    char *args[] = {"cat", "/proc/cpuinfo", NULL};
    spawn("cat", args);
}
