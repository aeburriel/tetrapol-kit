#pragma once

#include <stdint.h>

typedef struct {
    uint8_t z;
    uint8_t y;
    uint16_t x;
} st_addr_t;

struct hdlc_frame {
    st_addr_t addr;
    uint8_t command;
    uint8_t information[1];
    int information_len;
};
