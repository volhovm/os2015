#ifndef BUFIO_C
#define BUFIO_C
#include <sys/types.h>

typedef int fd_t;
struct buf_t {
    void* data;
    size_t capacity;
    size_t size;
};

struct buf_t* buf_new(size_t capacity);
void buf_free(struct buf_t*);
size_t buf_capacity(struct buf_t*);
size_t buf_size(struct buf_t*);
ssize_t buf_fill(fd_t fd, struct buf_t* buf, size_t required);
ssize_t buf_flush(fd_t fd, struct buf_t *buf, size_t required);
// returns string length (≥0) if \n has been found, -1 otherwise
ssize_t buf_getline(fd_t fd, struct buf_t* buf, char* dest);
ssize_t buf_write(fd_t fd, struct buf_t* buf, char* src, size_t len);
#endif
