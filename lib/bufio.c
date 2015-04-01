#include <bufio.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

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
#ifdef DEBUG
    if (required > buf->capacity) abort();
#endif
    while (buf->size < required) {
        int got = read(fd, buf->data + buf_size(buf),
                       buf->capacity - buf->size);
        if (got == -1) return -1;
        if (got == 0) break;
        buf->size += got;
    }
    return buf->size;
}

ssize_t buf_flush(fd_t fd, struct buf_t *buf, size_t required) {
    chnl(buf);
    int prev_size = buf->size;
    int written = 0;
    while (written < required) {
        int got = write(fd, buf->data + written, prev_size - written);
        if (got == -1) return -1;
        if (got == 0) break;
        written += got;
    }
    memmove(buf->data, buf->data + written, prev_size - written);
    buf->size = prev_size - written;
    return buf->size - prev_size;
}
