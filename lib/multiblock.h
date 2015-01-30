#pragma once

#define FRAME_NO_UNKNOWN -1

// now only data frame, in future might comprise different types of frame
typedef struct {
    int frame_no;
    union {
        uint8_t data[74];
        uint8_t _tmpd[76];  // extra space, data frame have 2 stuffing bist
    };
    union {
        uint8_t err[74];
        uint8_t _tmpe[76];  // extra space, data frame have 2 stuffing bist
    };
} data_frame_t;

void multiblock_reset(void);
void multiblock_process(data_frame_t *df, int fn);
