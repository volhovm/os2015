#include <helpers.h>
#include <stdlib.h>
#include <stdio.h>

main() {
    char* args_raw1[] = {"find", "/", NULL};
    char* args_raw2[] = {"sleep", "5", NULL};
    struct execargs_t* programs[2];
    programs[0] = execargs_fromargs(args_raw1);
    programs[1] = execargs_fromargs(args_raw2);
    int res = runpiped(programs, 2);
    return res;
}
