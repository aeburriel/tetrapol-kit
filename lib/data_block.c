#include "data_block.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

// http://ghsi.de/CRC/index.php?Polynom=10010
static void mk_crc5(uint8_t *res, const uint8_t *input, int input_len)
{
    uint8_t inv;
    memset(res, 0, 5);

    for (int i = 0; i < input_len; ++i)
    {
        inv = input[i] ^ res[0];

        res[0] = res[1];
        res[1] = res[2];
        res[2] = res[3] ^ inv;
        res[3] = res[4];
        res[4] = inv;
    }
}

/**
  PAS 0001-2 6.1.2
  PAS 0001-2 6.2.2
*/
static int decode_data_frame(uint8_t *res, uint8_t *err, const uint8_t *in, int res_len)
{
#ifdef GET_IN_
#error "Collision in definition of macro GET_IN_!"
#endif
#define GET_IN_(x, y) in[((x) + (y)) % (2*res_len)]

    int errs = 0;
    for (int i = 0; i < res_len; ++i) {
        res[i] = GET_IN_(2*i, 2) ^ GET_IN_(2*i, 3);
        err[i] = GET_IN_(2*i, 5) ^ GET_IN_(2*i, 6) ^ GET_IN_(2*i, 7);

        // we have 2 solutions, if match set to 0, 1 othervise
        err[i] ^= res[i];
        errs += err[i];
    }
#undef GET_IN_

    return errs;
}

void data_block_decode_frame(data_block_t *data_blk, const uint8_t *data,
        int frame_no, frame_type_t fr_type)
{
    data_blk->frame_no = frame_no;
    data_blk->nerrs = 0;

    if (fr_type == FRAME_TYPE_AUTO) {
        // TODO: try decode each type of frame
        fr_type = FRAME_TYPE_DATA;
    }

    if (fr_type == FRAME_TYPE_VOICE) {
        // TODO (set fr_type = FRAME_TYPE_DATA) when stollen frame
        printf("decoding frame type %d not implemented\n", fr_type);
        data_blk->nerrs = INT_MAX;
    }

    data_blk->fr_type = fr_type;

    if (fr_type == FRAME_TYPE_DATA) {
        // decode first 52 bites of frame
        data_blk->nerrs = decode_data_frame(
                data_blk->data, data_blk->err, data, 26);
        // decode remaining part of frame
        data_blk->nerrs += decode_data_frame(
                data_blk->data + 26, data_blk->err + 26, data + 2*26, 50);
    } else if (fr_type == FRAME_TYPE_HR_DATA) {
        // TODO
        printf("decoding frame type %d not implemented\n", fr_type);
        data_blk->nerrs = INT_MAX;
    } else {
        // TODO
        printf("decoding frame type %d not implemented\n", fr_type);
        data_blk->nerrs = INT_MAX;
    }
}

bool data_block_check_crc(data_block_t *data_blk)
{
    if (data_blk->fr_type == FRAME_TYPE_AUTO) {
        data_blk->fr_type = data_blk->data[0];
    } else {
        if (data_blk->fr_type != data_blk->data[0]) {
            return false;
        }
    }

    if (data_blk->fr_type == FRAME_TYPE_DATA) {
        uint8_t crc[5];

        mk_crc5(crc, data_blk->data, 69);
        return !memcmp(data_blk->data + 69, crc, 5);
    }

    if (data_blk->fr_type == FRAME_TYPE_VOICE) {
        // TODO
        fprintf(stderr, "CRC checking for VOICE frames not implemented");
        return false;
    }
    return false;
}

