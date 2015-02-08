#include "addr.h"

#include <stdio.h>

const addr_t addr_cgi_all_st = {
    .z = 0,
    .y = 0,
    .x = 0xfff,
};

const addr_t addr_tti_all_st = {
    .z = 0,
    .y = 7,
    .x = 0xfff,
};

const addr_t addr_tti_no_st = {
    .z = 0,
    .y = 7,
    .x = 0,
};

const addr_t addr_coi_all_st = {
    .z = 0,
    .y = 1,
    .x = 0, ///< 0 for all stations? it is not a bug in specification?
};

void addr_print(const addr_t *addr)
{
    printf("ADDR=%d.%d.%d", addr->z, addr->y, addr->x);
}

