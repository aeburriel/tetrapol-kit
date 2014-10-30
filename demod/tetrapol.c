#include "frame.h"
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
// max error rate for 2 frame synchronization sequences
#define MAX_FRAME_SYNC_ERR 1

// set on SIGINT
volatile static int do_exit = 0;
int mod;

static const uint8_t frame_sync[] = { 0, 1, 1, 0, 0, 0, 1, 0 };
#define FRAME_SYNC_LEN ((sizeof(frame_sync)))

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

static int process_frame(frame_t *frame)
{
    return 0;
}

// compare bite stream to differentialy encoded synchronization sequence
static int cmp_frame_sync(const uint8_t *buf, int invert)
{
    const uint8_t frame_dsync_pos[] = { 1, 0, 1, 0, 0, 1, 1, };
    const uint8_t frame_dsync_neg[] = { 0, 1, 0, 1, 1, 0, 0, };
    const uint8_t *frame_dsync = invert ? frame_dsync_neg : frame_dsync_pos;
    int sync_err = 0;
    for(int i = 0; i < sizeof(frame_dsync_pos); ++i) {
        if (frame_dsync[i] != buf[i + 1]) {
            ++sync_err;
        }
    }
    return sync_err;
}

/**
  Find 2 consecutive frame synchronization sequences.

  Using raw stream (before differential decoding) simplyfies search
  because only signal polarity must be considered,
  there is lot of troubles with error handlig after differential decoding.
  */
static int find_frame_sync(tetrapol_t *t)
{
    int offs = 0;
    int sync_err = MAX_FRAME_SYNC_ERR + 1;
    int invert = 0;
    while (offs + FRAME_LEN + FRAME_SYNC_LEN < t->data_len) {
        invert = 0;
        const uint8_t *buf = t->buf + offs;
        sync_err = cmp_frame_sync(buf, invert) +
                cmp_frame_sync(buf + FRAME_LEN, invert);
        if (sync_err <= MAX_FRAME_SYNC_ERR) {
            break;
        }

        invert = 1;
        sync_err = cmp_frame_sync(buf, invert) +
                cmp_frame_sync(buf + FRAME_LEN, invert);
        if (sync_err <= MAX_FRAME_SYNC_ERR) {
            break;
        }

        ++offs;
    }

    t->data_len -= offs;
    memmove(t->buf, t->buf + offs, t->data_len);

    if (sync_err <= MAX_FRAME_SYNC_ERR) {
        t->last_sync_err = 0;
        t->total_sync_err = 0;
        t->invert = invert;
        return 1;
    }

    return 0;
}

/// return number of acquired frames (0 or 1) or -1 on error
static int get_frame(frame_t *frame, tetrapol_t *t)
{
    if (t->data_len < FRAME_LEN) {
        return 0;
    }
    const int sync_err = cmp_frame_sync(t->buf, t->invert);
    if (sync_err + t->last_sync_err > MAX_FRAME_SYNC_ERR) {
        t->total_sync_err = 1 + 2 * t->total_sync_err;
        if (t->total_sync_err >= FRAME_LEN) {
            return -1;
        }
    } else {
        t->total_sync_err = 0;
    }

    t->last_sync_err = sync_err;
    memcpy(frame->data, t->buf, FRAME_LEN);
    // previous input bite for differential decoding is
    // defined only by signal polarity, ignore data from sync bits
    differential_dec(frame->data + FRAME_SYNC_LEN,
            FRAME_LEN - FRAME_SYNC_LEN, t->invert);
    // copy header, decoding it from frame leads only to errors
    memcpy(frame->data, frame_sync, sizeof(frame_sync));
    t->data_len -= FRAME_LEN;
    memmove(t->buf, t->buf + FRAME_LEN, t->data_len);

    return 1;
}

int tetrapol_main(tetrapol_t *t)
{
    signal(SIGINT, sigint_handler);

    while (!do_exit) {
        int recv = tetrapol_recv(t);
        if (recv <= 0) {
            return recv;
        }

        if (find_frame_sync(t)) {
            int r = 1;

            multiblock_reset();
            segmentation_reset();

            while (r >= 0 && !do_exit) {
                frame_t frame;

                while ((r = get_frame(&frame, t)) > 0) {
                    process_frame(&frame);
                }

                recv = tetrapol_recv(t);
                if (recv <= 0) {
                    return recv;
                }
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
