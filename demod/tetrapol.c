#include "constants.h"
#include "radio.h"
#include "multiblock.h"
#include "tpdu.h"
#include "tetrapol.h"

#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_LEN ((10*FRAME_LEN))

// set on SIGINT
volatile static int do_exit = 0;
int mod;


int tetrapol_init(tetrapol_t *t, int fd)
{
    memset(t, 0, sizeof(tetrapol_t));

    if (fcntl(fd, F_SETFL, O_NONBLOCK | fcntl(t->fd, F_GETFL))) {
        return -1;
    }
    t->fd = fd;

    t->buf = malloc(BUF_LEN);
    if (t->buf == NULL) {
        return -1;
    }

    radio_init();

    return 0;
}

void tetrapol_destroy(tetrapol_t *t)
{
    free(t->buf);
    t->buf = NULL;
}

static uint8_t differential_dec(uint8_t *buf, int size, uint8_t last_bit)
{
    while (size--) {
        last_bit = *buf = *buf ^ last_bit;
        ++buf;
    }
    return last_bit;
}

static void sigint_handler(int sig)
{
    do_exit = 1;
}

static int tetrapol_recv(tetrapol_t *t)
{
    struct pollfd fds;
    fds.fd = t->fd;
    fds.events = POLLIN;
    fds.revents = 0;

    if (poll(&fds, 1, -1) > 0 && !do_exit) {
        if (! (fds.revents & POLLIN)) {
            return -1;
        }
        int rsize = read(t->fd, t->buf + t->data_len, BUF_LEN - t->data_len);
        if (rsize == -1) {
            return -1;
        }
        t->data_len += rsize;

        return rsize;
    }

    return do_exit ? 0 : -1;
}

static int process_frame(tetrapol_t *t)
{
    return 0;
}

static int frame_sync(tetrapol_t *t)
{
    // TODO:
    // ! complete frame?  return 0
    // synchronize superframe
    // process frame in superframe
    return 0;
}

int tetrapol_main(tetrapol_t *t)
{
    signal(SIGINT, sigint_handler);

    while (!do_exit) {
        int recv = tetrapol_recv(t);
        if (recv <= 0) {
            return recv;
        }

        while (frame_sync(t)) {
            while (process_frame(t))
                ;;

            recv = tetrapol_recv(t);
            if (recv <= 0) {
                return recv;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    tetrapol_t t;
    if (tetrapol_init(&t, STDIN_FILENO) == -1) {
        return -1;
    }

    const int ret = tetrapol_main(&t);
    tetrapol_destroy(&t);

    fprintf(stderr, "Exiting.\n");

    return ret;
}

void mod_set(int m) {
	
	mod=m;
}
