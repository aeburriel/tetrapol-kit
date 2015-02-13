#include "tetrapol/bit_utils.h"

bool check_fcs(const uint8_t *data, int nbits)
{
    // roll in firts 16 bites of data
    uint32_t crc = 0;
    uint8_t b = data[0];
    for (int i = 0; i < 8; ++i) {
        crc = (crc << 1) | (b & 1);
        b = b >> 1;
    }
    b = data[1];
    for (int i = 0; i < 8; ++i) {
        crc = (crc << 1) | (b & 1);
        b = b >> 1;
    }

    // invert first 16 bits of data
    crc ^= 0xffff;

    nbits -= 16;
    data += 2;
    for ( ; nbits > 0; ++data) {
        b = *data;
        for (int offs = 0; offs < 8 && nbits; ++offs, --nbits) {
            // shift data bits into CRC
            crc = (crc << 1) | (b & 1);
            b = b >> 1;
            if (crc & 0x10000) {
                // CRC with poly: x^16 + x^12 + x^5 + 1
                crc ^= 0x11021;
            }
        }
    }

    return !(crc ^ 0xffff);
}
