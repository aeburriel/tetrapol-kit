#include "hdlc_frame.h"

#include <stdbool.h>

const st_addr_t st_addr_all = {
    .z = 0,
    .y = 7,
    .x = 0xfff,
};

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

// get int from byte array (TETRAPOL data frame)
static inline uint32_t struct_get_int(const uint8_t *data, int offs, int len)
{
    uint64_t r = 0;

    // collect all bites including those extra at begin/end
    for (int i = offs / 8; i < (offs + len + 7) / 8; ++i) {
        r = (r << 8) | data[i];
    }
    // drop extra bites at end
    r >>= 7 - ((len + offs - 1) % 8);
    // mask extra bites at begining
    r &= ((uint32_t)(~0L)) >> (32 - len);

    return r;
}

static void st_addr_parse(st_addr_t *addr, uint8_t *buf)
{
    addr->z = struct_get_int(buf, 0, 1);
    addr->y = struct_get_int(buf, 1, 3);
    addr->x = struct_get_int(buf, 4, 12);
}

// converts array of bits into bytes, uses TETRAPOL bite order
static void pack_bits(uint8_t *bytes, const uint8_t *bits, int nbits)
{
    int nbytes = nbits / 8;
    for (int i = 0; i < nbytes; ++i) {
        bytes[i] =
            (bits[8*i + 0] << 0) |
            (bits[8*i + 1] << 1) |
            (bits[8*i + 2] << 2) |
            (bits[8*i + 3] << 3) |
            (bits[8*i + 4] << 4) |
            (bits[8*i + 5] << 5) |
            (bits[8*i + 6] << 6) |
            (bits[8*i + 7] << 7);
    }
    nbits %= 8;
    if (nbits) {
        bytes[nbytes] = 0;
        for (int i = 0; i < nbits; ++i) {
            bytes[nbytes] |= (bits[8*nbytes + i] << i);
        }
    }
}

bool hdlc_frame_parse(hdlc_frame_t *hdlc_frame, const uint8_t *data, int len)
{
    if (!check_fcs(data, len)) {
        return false;
    }

    uint8_t buf[3];
    pack_bits(buf, data, 3*8);

    st_addr_parse(&hdlc_frame->addr, buf);
    // TODO: proper command parsing
    hdlc_frame->command = buf[2];

    pack_bits(hdlc_frame->info, data + 3*8, len - 3*8 - 2*8);
    // len - HDLC_header_len - FCS_len
    hdlc_frame->info_nbits = len - 3*8 - 2*8;

    return true;
}

