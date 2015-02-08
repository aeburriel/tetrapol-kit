#include "system_config.h"
#include "data_frame.h"
#include "misc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    FN_00 = 00,
    FN_01 = 01,
    FN_10 = 02,
    FN_11 = 03,
};

struct _data_frame_t {
    data_block_t data_blks[SYS_PAR_DATA_FRAME_BLOCKS_MAX + 1];
    int fn[SYS_PAR_DATA_FRAME_BLOCKS_MAX + 1];
    bool crc_ok[SYS_PAR_DATA_FRAME_BLOCKS_MAX + 1];
    int nblks;
    int nerrs;
};

data_frame_t *data_frame_create(void)
{
    data_frame_t *data_fr = malloc(sizeof(data_frame_t));
    if (!data_fr) {
        return NULL;
    }

    data_frame_reset(data_fr);

    return data_fr;
}

void data_frame_destroy(data_frame_t *data_fr)
{
    free(data_fr);
}

int data_frame_blocks(data_frame_t *data_fr)
{
    return data_fr->nblks;
}

void data_frame_reset(data_frame_t *data_fr)
{
    data_fr->nblks = 0;
    data_fr->nerrs = 0;
}

static bool check_parity(data_frame_t *data_fr)
{
    for (int i = 1; i < 1 + 64 + 2; ++i) {
        int parity = 0;
        for (int blk_no = 0; blk_no < data_fr->nblks; ++blk_no) {
            parity ^= data_fr->data_blks[blk_no].data[i];
        }
        if (parity) {
            return false;
        }
    }

    return true;
}

static void fix_by_parity(data_frame_t *data_fr)
{
    int err_blk_no = 0;

    for (int blk_no = 0; blk_no < data_fr->nblks; ++blk_no) {
        if (!data_fr->crc_ok[blk_no]) {
            err_blk_no = blk_no;
            break;
        }
    }

    // do not fix parity frame
    if (err_blk_no == data_fr->nblks - 1) {
        return;
    }

    for (int i = 1; i < 1 + 64 + 2; ++i) {
        int bit = 0;
        for (int blk_no = 0; blk_no < data_fr->nblks; ++blk_no) {
            if (blk_no != err_blk_no) {
                bit ^= data_fr->data_blks[blk_no].data[i];
            }
            data_fr->data_blks[err_blk_no].data[i] = bit;
        }
    }
}

static bool data_frame_check_multiblock(data_frame_t *data_fr)
{
    if (data_fr->nerrs) {
        fix_by_parity(data_fr);
    } else {
        if (!check_parity(data_fr)) {
            printf("MB parity error %d\n", data_fr->nblks);
            data_frame_reset(data_fr);
            return false;
        }
    }

    return true;
}

bool data_frame_push_data_block(data_frame_t *data_fr, data_block_t *data_blk)
{
    if (data_fr->nblks == ARRAY_LEN(data_fr->data_blks)) {
        data_frame_reset(data_fr);
    }

    const bool crc_ok = data_block_check_crc(data_blk) && !data_blk->nerrs;
    data_fr->nerrs += crc_ok ? 0 : 1;
    data_fr->crc_ok[data_fr->nblks] = crc_ok;

    if (data_fr->nerrs > 1) {
        data_frame_reset(data_fr);
        return false;
    }

    const int fn = data_blk->data[1] | (data_blk->data[2] << 1);
    data_fr->fn[data_fr->nblks] = fn;

    memcpy(&data_fr->data_blks[data_fr->nblks], data_blk, sizeof(data_block_t));
    ++data_fr->nblks;

    // single frame
    if (data_fr->nblks == 1) {
        if (!crc_ok) {
            return false;
        }
        if (fn == FN_00) {
            return true;
        }
        if (fn != FN_01) {
            printf("MB err\n");
            data_frame_reset(data_fr);
        }
        return false;
    }

    const int fn_prev = data_fr->fn[data_fr->nblks - 2];
    const bool crc_ok_prev = data_fr->crc_ok[data_fr->nblks - 2];

    // check for dualframe or multiframe
    if (data_fr->nblks == 2) {
        if (!crc_ok) {
            if (fn_prev != FN_01) {
                data_frame_reset(data_fr);
            }
            return false;
        }
        if (fn == FN_11) {
            if (!crc_ok_prev) {
                data_frame_reset(data_fr);
                return false;
            }
            return true;
        }
        if (fn != FN_10) {
            printf("MB err\n");
            data_frame_reset(data_fr);
            return data_frame_push_data_block(data_fr, data_blk);
        }
        return false;
    }

    // check multiframe, inner frames
    if (data_fr->nblks == 3) {
        if (!crc_ok) {
            return false;
        }
        if (fn != FN_10 && fn != FN_11) {
            data_frame_reset(data_fr);
            return data_frame_push_data_block(data_fr, data_blk);
        }
        return false;
    }

    // end of multiframe, final frame is invalid
    if (!crc_ok) {
        if (fn_prev == FN_10) {
            return data_frame_check_multiblock(data_fr);
        }
        return false;
    }

    if (fn == FN_11) {
        if (fn_prev != FN_11 && crc_ok_prev) {
            data_frame_reset(data_fr);
        }
        return false;
    }

    // check multiframe, pre-end of multiframe
    if (fn == FN_10) {
        if (fn_prev != FN_11 && crc_ok_prev) {
            data_frame_reset(data_fr);
        }
        return false;
    }

    if (fn == FN_01) {
        if (fn_prev != FN_10 && crc_ok_prev) {
            data_frame_reset(data_fr);
            return false;
        }
        return data_frame_check_multiblock(data_fr);
    }

    printf("MB err\n");
    data_frame_reset(data_fr);
    return data_frame_push_data_block(data_fr, data_blk);
}

/**
  Pack bites from one bit per byte into 8 bites per byte (TETRAPOL ite order).

  Data are ORed, so do not forrget to initialise output buffer with zeres.

  @param bytes Output byte array.
  @param bits Input array of bits.
  @param offs Number of bites already used in output.
  @param nbits Number of bites to be used;
  */
static void pack_bits(uint8_t *bytes, const uint8_t *bits, int offs, int nbits)
{
    bytes += offs / 8;
    offs %= 8;

    while (nbits > 0) {
        while (offs < 8 && nbits) {
            *bytes |= (*bits) << offs;
            ++offs;
            --nbits;
            ++bits;
        }
        offs = 0;
        ++bytes;
    }
}

int data_frame_get_bytes(data_frame_t *data_fr, uint8_t *data)
{
    const int nblks = (data_fr->nblks <= 2) ?
        data_fr->nblks : data_fr->nblks - 1;

    memset(data, 0, 8*nblks);
    for (int blk_no = 0; blk_no < nblks; ++blk_no) {
        pack_bits(data, data_fr->data_blks[blk_no].data + 3, 64*blk_no, 64);
    }

    data_frame_reset(data_fr);

    return nblks * 64;
}
