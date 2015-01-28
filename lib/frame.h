#pragma once

#include <stdint.h>

// code expects it is aligned to byte boundary
#define FRAME_HDR_LEN (8)
#define FRAME_DATA_LEN (152)
#define FRAME_LEN (FRAME_HDR_LEN + FRAME_DATA_LEN)

typedef struct {
    int frame_no;
    uint8_t data[FRAME_DATA_LEN];
} frame_t;

