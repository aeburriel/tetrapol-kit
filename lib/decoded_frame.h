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
    int frame_no;
    union {
        uint8_t data[74];
        uint8_t _tmpd[76];  // extra space, data frame have 2 stuffing bist
    };
    union {
        uint8_t err[74];
        uint8_t _tmpe[76];  // extra space, data frame have 2 stuffing bist
    };
} decoded_frame_t;

bool decoded_frame_check_crc(const decoded_frame_t *df, frame_type_t df_type);

