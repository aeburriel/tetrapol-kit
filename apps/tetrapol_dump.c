#include "tetrapol.h"
// TODO: should use only tetrapol.h, but hi-level interface not implemented yet
#include "phys_ch.h"

#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// set on SIGINT
volatile static int do_exit = 0;


static void sigint_handler(int sig)
{
    do_exit = 1;
}

static int do_read(int fd, uint8_t *buf, int len)
{
    struct pollfd fds;
    fds.fd = fd;
    fds.events = POLLIN;
    fds.revents = 0;

    if (poll(&fds, 1, -1) > 0 && !do_exit) {
        if (! (fds.revents & POLLIN)) {
            return -1;
        }

        return read(fd, buf, len);
    }

    return do_exit ? 0 : -1;
}

static int tetrapol_dump_loop(phys_ch_t *phys_ch, int fd)
{
    int ret = 0;
    int data_len = 0;
    uint8_t data[4096];

    if (fcntl(fd, F_SETFL, O_NONBLOCK | fcntl(fd, F_GETFL))) {
        return -1;
    }

    signal(SIGINT, sigint_handler);

    while (ret == 0 && !do_exit) {
        if (sizeof(data) - data_len > 0) {
            const int rsize = do_read(fd, data + data_len, sizeof(data) - data_len);
            if (rsize <= 0) {
                return rsize;
            }
            data_len += rsize;
        }

        const int rsize = tetrapol_recv2(phys_ch, data, data_len);
        if (rsize < 0) {
            return rsize;
        }
        if (rsize > 0) {
            memmove(data, data + rsize, data_len - rsize);
            data_len -= rsize;
        }

        ret = tetrapol_phys_ch_process(phys_ch);
    }

    return ret;
}


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

    phys_ch_t *phys_ch = tetrapol_phys_ch_create();
    if (phys_ch == NULL) {
        fprintf(stderr, "Failed to initialize TETRAPOL instance.");
        return -1;
    }

    const int ret = tetrapol_dump_loop(phys_ch, infd);
    tetrapol_phys_ch_destroy(phys_ch);
    if (infd != STDIN_FILENO) {
        close(infd);
    }

    fprintf(stderr, "Exiting.\n");

    return ret;
}
