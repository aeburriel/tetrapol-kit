#include "tetrapol.h"

#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    const char *in = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "i:")) != -1) {
        switch (opt) {
            case 'i':
                in = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [-i IN_FILE_PATH]\n", argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    int infd = STDIN_FILENO;
    if (in && strcmp(in, "-")) {
        infd = open(in, O_RDONLY);
        if (infd == -1) {
            perror("Failed to open input file");
            return -1;
        }
    }

    tetrapol_t t;
    if (tetrapol_init(&t, infd) == -1) {
        return -1;
    }

    const int ret = tetrapol_main(&t);
    tetrapol_destroy(&t);
    if (infd != STDIN_FILENO) {
        close(infd);
    }

    fprintf(stderr, "Exiting.\n");

    return ret;
}
