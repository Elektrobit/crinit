#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
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

    const size_t chunk_size = 1024 * 1024;
    const unsigned int chunk_count = 500;
    char *memory_block[chunk_count];

    printf("Starting memory allocation off %d MB in chunks of %zu bytes...\n", chunk_count, chunk_size);

    for (unsigned int i = 0; i < chunk_count; i++) {
        memory_block[i] = malloc(chunk_size);
        if (memory_block[i] == NULL) {
            printf("Failed to allocate memory. Wanted to allocate chunk number %d.\n", i + 1);

            for (unsigned int j = 0; j < i; j++) {
                free(memory_block[j]);
                return 1;
            }
        }
        for (unsigned int k = 0; k < chunk_size; k++) {
            memory_block[i][k] = rand() & 0xFF;
        }
    }

    printf("Allocated %d MB memory successfully.\n", chunk_count);
    for (unsigned int i = 0; i < chunk_count; i++) {
        free(memory_block[i]);
    }

    return 0;
}
