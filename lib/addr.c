#include "addr.h"

#include <stdio.h>

void addr_print(const addr_t *addr)
{
    printf("ADDR=%d.%d.0x%03x", addr->z, addr->y, addr->x);
}

