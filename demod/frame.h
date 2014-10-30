#pragma once
#ifndef FRAME_H
#define FRAME_H

#include <stdint.h>

// code expects it is aligned to byte boundary
#define FRAME_LEN	160

typedef struct {
    int frame_no;
    uint8_t data[FRAME_LEN];
} frame_t;

#endif
