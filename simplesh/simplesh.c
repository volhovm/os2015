#include <helpers.h>
#include <stdlib.h>
#include <stdio.h>

main() {
    char* args_raw1[] = {"sleep", "5", NULL};
    char* args_raw2[] = {"echo", "mda..heh", NULL};
    char* args_raw3[] = {"grep", "heh", NULL};
    struct execargs_t* programs[3];
    programs[0] = execargs_fromargs(args_raw1);
    programs[1] = execargs_fromargs(args_raw2);
    programs[2] = execargs_fromargs(args_raw3);
    int res = runpiped(programs, 3);
    return res;
}
