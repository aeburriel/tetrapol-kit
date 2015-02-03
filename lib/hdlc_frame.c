#include "hdlc_frame.h"

#include <stdbool.h>


/// PAS 0001-3-3 7.4.1.1
static bool check_fcs(const uint8_t *data, int len)
{
    uint8_t crc[16];

    // invert first 16 bits of data
    for (int i = 0; i < 16; ++i) {
        crc[i] = data[i] ^ 1;
    }

    // CRC with poly: x^16 + x^12 + x^5 + 1
    for (int i = 16; i < len; ++i) {
        int xor = crc[0];

        crc[0] = crc[1];
        crc[1] = crc[2];
        crc[2] = crc[3];
        crc[3] = crc[4] ^ xor;
        crc[4] = crc[5];
        crc[5] = crc[6];
        crc[6] = crc[7];
        crc[7] = crc[8];
        crc[8] = crc[9];
        crc[9] = crc[10];
        crc[10] = crc[11] ^ xor;
        crc[11] = crc[12];
        crc[12] = crc[13];
        crc[13] = crc[14];
        crc[14] = crc[15];
        // CRC at the end of frame is inverted, invert it again
        if (i >= len - 16) {
            crc[15] = data[i] ^ xor ^ 1;
        } else {
            crc[15] = data[i] ^ xor;
        }
    }

    return !(crc[0] | crc[1] | crc[2] | crc[3] | crc[4] | crc[5] | crc[6] | crc[7] |
            crc[8] | crc[9] | crc[10] | crc[11] | crc[12] | crc[13] | crc[14] | crc[15]);
}
