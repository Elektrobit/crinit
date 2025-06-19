#include <signal.h>
#include <string.h>

static volatile sig_atomic_t crinit_done = 0;

static void term_handler(int signum) {
    if (signum == SIGTERM) {
        crinit_done = 1;
    }
}

int main(int argc, char **argv) {
    (void)(argc);
    (void)(argv);
    struct sigaction sigfilter;
    memset(&sigfilter, 0x00, sizeof(sigfilter));
    sigfilter.sa_handler = term_handler;
    sigaction(SIGTERM, &sigfilter, NULL);

    unsigned int num = 1, divisor;

    for (;;) {
        divisor = 2;
        while (divisor <= num) {
            if (crinit_done > 0) {
                return 1;
            }
            if (num % divisor == 0) {
                break;
            }
            divisor++;
        }
        num++;
    }

    return 0;
}
