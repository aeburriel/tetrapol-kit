// this file should be directly included into another .c file

#include <stdint.h>

// get int from byte array (bytes of TETRAPOL data frame)
inline uint32_t struct_get_int(const uint8_t *data, int offs, int len)
{
    uint64_t r = 0;

    // collect all bits including those extra at begin/end
    for (int i = offs / 8; i < (offs + len + 7) / 8; ++i) {
        r = (r << 8) | data[i];
    }
    // drop extra bits at end
    r >>= 7 - ((len + offs - 1) % 8);
    // mask extra bits at begining
    r &= ((uint32_t)(~0L)) >> (32 - len);

    return r;
}

