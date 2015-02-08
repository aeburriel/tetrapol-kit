#pragma once

#include "bit_utils.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t z;
    uint8_t y;
    uint16_t x;
} addr_t;

inline bool addr_is_cgi_all_st(const addr_t *addr, bool z)
{
    if (z) {
        return addr->z == 0 && addr->y == 0 && addr->x == 0xfff;
    } else {
        return addr->y == 0 && addr->x == 0xfff;
    }
};

inline bool addr_is_tti_all_st(const addr_t *addr, bool z)
{
    if (z) {
        return addr->z == 0 && addr->y == 7 && addr->x == 0xfff;
    } else {
        return addr->y == 7 && addr->x == 0xfff;
    }
}

inline bool addr_is_tti_no_st(const addr_t *addr, bool z)
{
    if (z) {
        return addr->z == 0 && addr->y == 7 && addr->x == 0;
    } else {
        return addr->y == 7 && addr->x == 0;
    }
};

inline bool addr_is_coi_all_st(const addr_t *addr)
{
    return addr->y == 1 && addr->x == 0;
    // x=0 for all stations? it is not a bug in specification?
};

inline void addr_parse(addr_t *addr, uint8_t *buf)
{
    addr->z = get_bits(1, 0, buf);
    addr->y = get_bits(3, 1, buf);
    addr->x = get_bits(12, 4, buf);
}

void addr_print(const addr_t *addr);

