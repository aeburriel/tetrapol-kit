#pragma once

// this file should be directly included into another .c file

#include <stdbool.h>
#include <stdint.h>

/// PAS 0001-3-3 7.4.1.1
/**
 * Check TETRAPOL style FCS of the data block.
 *
 * @param data Data packed into bytes.
 * @param nbits Lenght of data in bites, not necesary multiple of 8
 * @return true if FCS at the end of block is correct, false otherwise.
 */
bool check_fcs(const uint8_t *data, int nbits);

/**
 * @brief get_bits Get int from byte array (bytes of TETRAPOL data frame).
 * @param len Bites used for extraction.
 * @param data
 * @param skip Bites skipped from beginning of data (counts from MSB, MSB = bit0)
 * @return Integer value.
 */
inline uint32_t get_bits(int len, const uint8_t *data, int skip)
{
    uint64_t r = 0;

    // collect all bits including those extra at begin/end
    for (int i = skip / 8; i < (skip + len + 7) / 8; ++i) {
        r = (r << 8) | data[i];
    }
    // drop extra bits at end
    r >>= 7 - ((len + skip - 1) % 8);
    // mask extra bits at begining
    r &= ((uint32_t)(~0L)) >> (32 - len);

    return r;
}

