#include <stdio.h>
#include <stdlib.h>
#include <bufio.h>

void chnl(struct buf_t* buf) {
#ifdef DEBUG
    if (buf == NULL) {
        abort();
    }
#endif
}

struct buf_t* buf_new(size_t capacity) {
    void* data = malloc(capacity);
    if (data == NULL) return NULL;
    void* space = malloc(sizeof(struct buf_t));
    if (space == NULL) return NULL;
    struct buf_t* buf = (struct buf_t*) space;
    buf->data = data;
    buf->capacity = capacity;
    buf->size = 0;
    return buf;
}

void buf_free(struct buf_t* buf) {
    chnl(buf);
    free(buf->data);
    free(buf);
}

size_t buf_capacity(struct buf_t* buf) {
    chnl(buf);
    return buf->capacity;
}

size_t buf_size(struct buf_t* buf) {
    chnl(buf);
    return buf->size;
}

ssize_t buf_fill(fd_t fd, struct buf_t* buf, size_t required) {
    chnl(buf);
    return 0;
}

ssize_t buf_flush(fd_t fd, struct buf_t *buf, size_t required) {
    chnl(buf);
    return -1;
}
