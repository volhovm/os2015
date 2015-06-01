#ifndef HELPERS_C
#define HELPERS_C
#include <sys/types.h>

ssize_t read_(int fd, void* buf, size_t count);
ssize_t write_(int fd, const void* buf, size_t count);
ssize_t read_until(int fd, void* buf, size_t count, char delimiter);
int spawn(const char* file, char* const argv []);

struct execargs_t {
    char* name;
    char** args;
};
struct execargs_t* execargs_new(char* name, char** args);
struct execargs_t* execargs_fromargs(char** args);
int exec(struct execargs_t* args);
int signals_first(const sigset_t* smask);
int signals_unblock(const sigset_t* smask);
int runpiped1(struct execargs_t** programs, size_t n, int write_to, sigset_t* smask);
int runpiped(struct execargs_t** programs, size_t n);
#endif
