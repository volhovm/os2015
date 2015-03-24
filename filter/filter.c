#include <helpers.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int BUFFER_SIZE = 4096;
char DIVIDER = '\n';

void spawn_without_io(char* file, int newargs, char* newarg, int argc, char* argv[]) {
    int stdoutdub, res;
    argv[argc - 2] = newarg;
    stdoutdub = dup(1);          // backup for stdout
    close(1);                    // fd 1 is free now
    open("/dev/null", O_WRONLY); // must take the fd 1
    res = spawn(file, argv);
    dup2(stdoutdub, 1);          // recover stdout
    //printf("\nCALLED SPAWN WITH %s, %d, %s, %d, 0:%s, 1:%s, 2:%s, %d\n", file,
    //       newargs, newarg, argc, argv[0], argv[1], argv[2], res);
    fflush(stdout);
    if (res == 0) {
        write(STDOUT_FILENO, argv[argc - 2], newargs);
        write(STDOUT_FILENO, " ", 1);
        fflush(stdout);
    }
}

void main(int argc, char* argv[]) {
    //char *argx[] = {"ls", "/bin/", NULL};
    //int res0 = spawn("ls", argx);
    //printf("Spawn returned %d", res0);
    //return;

    int i, got;
    char* util_name = argv[1];
    char* args[argc + 1];
    char buf[BUFFER_SIZE];
    for (i = 1; i < argc; i++) {
        args[i-1] = argv[i];
    }
    args[argc] = NULL;
    // args[argc - 1] is the place for n+1th argument

    int margin = 0;
    while (1) {
        got = read_until(STDIN_FILENO, buf + margin, BUFFER_SIZE / 4, DIVIDER);
        // Processing the last portion of data
        if (got == 0) {
            char arg[margin];
            memmove(arg, buf, margin - 1);
            arg[margin - 1] = 0;
            spawn_without_io(util_name, margin, arg, argc + 1, args);
            break;
        }
        int last_mark = 0;
        for (i = margin; i < margin + got; i++) {
            if (buf[i] == DIVIDER) {
                int tmpsize = i - last_mark + 1; // the size of new word
                char arg[tmpsize];
                memcpy(arg, buf + last_mark, tmpsize - 1);
                arg[tmpsize - 1] = 0;
                spawn_without_io(util_name, tmpsize - 1, arg, argc + 1, args);
                last_mark = i + 1;
            }
        }
        memmove(buf, buf + last_mark, got + margin - last_mark + 1);
        margin = got + margin - last_mark;
    }

}
