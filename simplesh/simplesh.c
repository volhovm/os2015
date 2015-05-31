#include <helpers.h>
#include <sys/types.h>
#include <unistd.h>
#include <bufio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main() {
    setbuf(stdout, NULL);

    //char* args_raw1[] = {"ls", "/bin", NULL};
    //char* args_raw2[] = {"sleep", "5", NULL};
    //struct execargs_t* pgs[1];
    //pgs[0] = execargs_fromargs(args_raw1);
    ////programs[1] = execargs_fromargs(args_raw2);
    //return runpiped(pgs, 1);


    int commands_n, args_n, res, i, j, terminated;
    char c;
    size_t got;
    struct buf_t* buf = buf_new(8192);
    char current[4096];
    char* token, * current_copy;
    char* commands[30]; // 30 commands at a time
    char* parsed[30][40]; // 30 commands, up to 39 arguments each
    struct execargs_t* programs[30];
    while (1){
        res = write(STDOUT_FILENO, "$", 1);
        if (res == -1) return -1;
        if (res == 0) return 0;
        commands_n = 0;
        got = buf_getline(STDIN_FILENO, buf, current);
        current_copy = strdup(current);
        if (got == 0) continue;
        while ((token = strsep(&current_copy, "|")) != NULL) {
            commands[commands_n++] = token;
        }
        printf("Scanned all programs, length: %d\n", commands_n);
        for (i = 0; i < commands_n; i++) {
            args_n = 0;
            j = 0;
            terminated = 1;
            for (j = 0; ; j++) {
                c = commands[i][j];
                if (c == 0) {
                    parsed[i][args_n+1] = NULL;
                    printf("parsing: program %d : %d args\n", i, args_n);
                    programs[i] = execargs_fromargs(parsed[i]);
                    if (!terminated) args_n++;
                    break;
                }
                if (c == ' ' && !terminated) {
                    args_n++;
                    terminated = 1;
                    commands[i][j] = 0; // terminate the string
                    continue;
                }
                if (c == ' ') continue;

                // start parsing new token
                if (terminated) {
                    terminated = 0;
                    parsed[i][args_n] = (char*) (commands[i] + j);
                }

                // it's ok
            }
        }
        res = runpiped(programs, commands_n);
        printf("exited with code %d\n", res);
    }
    return 0;
}
