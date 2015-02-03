#pragma once

#include <stdbool.h>
#include <stdint.h>

// 92 bits in high rate data frame, up-to 8 frames create data frame
#define HDLC_INFO_LEN_MAX   92

typedef struct {
    uint8_t z;
    uint8_t y;
    uint16_t x;
} st_addr_t;

typedef struct {
    st_addr_t addr;
    uint8_t command;
    int info_nbits;                     ///< lenght is in bits
    uint8_t info[HDLC_INFO_LEN_MAX];    ///< data are stored packed in bytes
} hdlc_frame_t;

bool hdlc_frame_parse(hdlc_frame_t *hdlc_frame, const uint8_t *data, int len);
