#pragma once
#ifndef TETRAPOL_H
#define TETRAPOL_H

#include <stdint.h>

typedef struct {
    uint8_t *buf;
    int data_len;
    int fd;
    int last_sync_err;  ///< errors in last frame synchronization sequence
    int total_sync_err; ///< cumulative error in framing
    int invert;         ///< polarity of differentialy encoded bits stream
} tetrapol_t;

int tetrapol_init(tetrapol_t *t, int fd);
void tetrapol_destroy(tetrapol_t *t);
int tetrapol_main(tetrapol_t *t);

#endif
