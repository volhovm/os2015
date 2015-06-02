#include <signal.h>
#include <unistd.h>

void f(int sig) {}

int main()
{
    struct sigaction ac;
    ac.sa_handler = &f;
    sigaction(SIGINT, &ac, NULL);
    while (1) {}
}
