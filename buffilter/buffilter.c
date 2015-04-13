#include <buffilter.h>
#include <bufio.h>
#include <helpers.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int BUFFER_SIZE = 4096;

// Calling spawn, temporarily routing stdin to /dev/null
void spawn_without_io(char* file, int newargs, char* newarg, int argc, char* argv[]) {
    int stdoutdub, res;
    argv[argc - 2] = newarg;
    stdoutdub = dup(1);          // backup for stdout
    close(1);                    // fd 1 is free now
    open("/dev/null", O_WRONLY); // must take the fd 1
    res = spawn(file, argv);
    dup2(stdoutdub, 1);          // recover stdout
    fflush(stdout);
    //    printf("spawned %s with %s of size %d, got %d", file, newarg, newargs, res);
    if (res == 0) {
        write(STDOUT_FILENO, newarg, newargs);
        write(STDOUT_FILENO, " ", 1);
        fflush(stdout);
    }
}

void main(int argc, char* argv[]) {
    int i, got;
    char* util_name = argv[1];
    char* args[argc + 1];
    char last_arg[4096];
    struct buf_t* buf = buf_new(BUFFER_SIZE);
    for (i = 1; i < argc; i++) {
        args[i-1] = argv[i];
    }
    args[argc] = NULL;
    // args[argc - 1] is the place for n+1th argument

    while (1) {
        got = buf_getline(STDIN_FILENO, buf, last_arg);
        //printf("SPAWNING\n");
        if (got == 0) break;
        spawn_without_io(util_name, strlen(last_arg), last_arg, argc + 1, args);
    }
}
