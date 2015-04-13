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
void spawn_without_io(char* file, char* newarg, int argc, char* argv[], struct buf_t* buf) {
    int stdoutdub, res;
    argv[argc - 2] = newarg;
    stdoutdub = dup(1);          // backup for stdout
    int other = open("/dev/null", O_WRONLY); // must take the fd 1
    dup2(other, 1);
    res = spawn(file, argv);
    dup2(stdoutdub, 1);          // recover stdout
    close(other);
    close(stdoutdub);
    //    printf("spawned %s with %s of size %d, got %d", file, newarg, newargs, res);
    if (res == 0) {
        buf_write(STDOUT_FILENO, buf, newarg, strlen(newarg));
        buf_write(STDOUT_FILENO, buf, "\n", 1);
    }
}

void main(int argc, char* argv[]) {
    int i, got;
    char* util_name = argv[1];
    char* args[argc + 1];
    char last_arg[4096];
    struct buf_t* bufin = buf_new(BUFFER_SIZE);
    struct buf_t* bufout = buf_new(BUFFER_SIZE);
    for (i = 1; i < argc; i++) {
        args[i-1] = argv[i];
    }
    args[argc] = NULL;
    // args[argc - 1] is the place for n+1th argument

    while (1) {
        got = buf_getline(STDIN_FILENO, bufin, last_arg);
        if (got == 0) break;
        spawn_without_io(util_name, last_arg, argc + 1, args, bufout);
    }
    buf_flush(STDOUT_FILENO, bufout, buf_size(bufout));
    buf_free(bufin);
    buf_free(bufout);
}
