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

ssize_t buf_getline(fd_t fd, struct buf_t* buf, char* dest) {
    int i, size, got;
    int start = 0;
    size = buf->size;
    while (buf->size < buf->capacity) {
        for (i = start; i < buf->size; i++) {
            if (((char*) buf->data)[i] == '\n') {
                memmove(dest, buf->data, i);
                dest[i] = 0;
                memmove(buf->data, buf->data + i + 1, buf->size - i - 1);
                buf->size = buf->size - i - 1;
                return i;
            }
        }
        got = buf_fill(fd, buf, 1);
        if (got == 0) return 1;
        if (size == got) {
            memmove(dest, buf->data, buf->size);
            dest[got] = 0;
            buf->size = 0;
            return got;
        }
        start = size;
        size = buf->size;
    }
    return -1;
}

ssize_t buf_write(fd_t fd, struct buf_t* buf, char* src, size_t len) {
    int newlen, res;
    while (1) {
        if (buf->size + len < buf->capacity) {
            memmove(buf->data + buf->size, src, len);
            buf->size = buf->size + len;
            return len;
        }
        newlen = buf->size - len;
        memmove(buf->data + buf->size, src, newlen);
        buf->size = buf->capacity;
        src += newlen;
        len -= newlen;
        res = buf_flush(fd, buf, 1);
        if (res < 0) return -1;
    }
}
