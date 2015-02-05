#pragma once

#include <stdbool.h>
#include <stdint.h>

#define FRAME_NO_UNKNOWN -1

typedef enum {
    FRAME_TYPE_AUTO = -1,
    FRAME_TYPE_VOICE = 0,
    FRAME_TYPE_DATA = 1,
    FRAME_TYPE_HR_DATA,
    FRAME_TYPE_RANDOM_ACCESS,
    FRAME_TYPE_TRAINING,
    FRAME_TYPE_DM_EMERGENCY,
    FRAME_TYPE_SCH_TI,
} frame_type_t;

// now only data frame, in future might comprise different types of frame
typedef struct {
    frame_type_t fr_type;
    int frame_no;
    // 74 bytes is required for data frame, but 2 extra bits are reqired
    // because they are used by decoding algorithm
    // 126 bits for voice frame
    // TODO: 152 bits for high rate data frames? (or use BCH and reduce it to 96)
    // TODO: 152 bits for RACH frames?
    // TODO: 152 bits for training frame?
    // TODO: 152 bits for SCH/TI frame?
    uint8_t data[126];
    union {
        uint8_t err[74];
        uint8_t _tmpe[76];  // extra space, data frame have 2 stuffing bist
    };
} data_block_t;

bool data_block_check_crc(data_block_t *data_blk);

