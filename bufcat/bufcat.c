#include <bufio.h>
#include <unistd.h>

const int CAPACITY = 4096;

int main() {
    struct buf_t* buf = buf_new(CAPACITY);
    int res1, res2;
    while (1) {
        res1 = buf_fill(STDIN_FILENO, buf, 1);
        res2 = buf_flush(STDOUT_FILENO, buf, buf_size(buf));
        if (res1 == -1 || res2 == -1) break;
    }
    buf_free(buf);
    return 0;
}
