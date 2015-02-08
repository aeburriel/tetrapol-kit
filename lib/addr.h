#pragma once

#include "bit_utils.h"

#include <stdint.h>

typedef struct {
    uint8_t z;
    uint8_t y;
    uint16_t x;
} addr_t;

extern const addr_t addr_cgi_all_st;
extern const addr_t addr_coi_all_st;
extern const addr_t addr_tti_no_st;
extern const addr_t addr_tti_all_st;

inline void addr_parse(addr_t *addr, uint8_t *buf)
{
    addr->z = get_bits(1, 0, buf);
    addr->y = get_bits(3, 1, buf);
    addr->x = get_bits(12, 4, buf);
}

void addr_print(const addr_t *addr);

