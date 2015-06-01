#include <helpers.h>
#include <sys/types.h>
#include <unistd.h>
#include <bufio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int running = 1;

int main() {
    setbuf(stdout, NULL);
    setbuf(stdin, NULL);

    // block sigint
    sigset_t smask;
    sigemptyset(&smask);
    sigaddset(&smask, SIGINT);
    sigprocmask(SIG_BLOCK, &smask, NULL);

    //char* args_raw1[] = {"sleep", "5", NULL};
    ////char* args_raw2[] = {"sleep", "5", NULL};
    //struct execargs_t* pgs[10];
    //pgs[0] = execargs_fromargs(args_raw1);
    ////programs[1] = execargs_fromargs(args_raw2);
    //int reslt = runpiped(pgs, 1);
    //printf("retcode: %d", reslt);
    //return reslt;


    int commands_n, args_n, res, i, j, terminated, got;
    char c;
    struct buf_t* buf = buf_new(8192);
    char current[4096];
    char* token, * current_copy;
    char* commands[30]; // 30 commands at a time
    char* parsed[30][40]; // 30 commands, up to 39 arguments each
    struct execargs_t* programs[30];
    while (running) {
        res = write(STDOUT_FILENO, "$ ", 2);
        if (res == -1 || res == 0) { // something is utterly wrong
            signals_unblock(&smask);
            return -1;
        }
        commands_n = 0;
        got = buf_getline(STDIN_FILENO, buf, current);
        //got = read(STDIN_FILENO, buf, 20);
        current_copy = strdup(current);
        printf("got %d read\n", got);
        if (got == -1) {
            signals_unblock(&smask);
            return -1;
        }
        if (got == 0) continue;
        if (got == 1) { // that's eof
            break;
        }
        while ((token = strsep(&current_copy, "|")) != NULL) {
            commands[commands_n++] = token;
        }
        printf("Scanned all programs, length: %d\n", commands_n);
        for (i = 0; i < commands_n; i++) {
            args_n = 0;
            current_copy = strdup(commands[i]);
            while ((token = strsep(&current_copy, " ")) != NULL) {
                if (strlen(token) == 0) continue;
                parsed[i][args_n++] = token;
            }
            parsed[i][args_n] = NULL; // terminate the sequence
            programs[i] = execargs_fromargs(parsed[i]);
            printf("parsing: programm #%d, tokens: %d\n", i, args_n);
        }

        res = runpiped(programs, commands_n);
        printf("exited with code %d\n", res);
    }
    signals_unblock(&smask);
    return 0;
}
