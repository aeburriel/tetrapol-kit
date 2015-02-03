#include "data_frame.h"
#include "misc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// maximal amount of data frames including single XOR frame
#define MAX_DATA_FRAMES 9

enum {
    FN_00 = 00,
    FN_01 = 01,
    FN_10 = 02,
    FN_11 = 03,
};

struct _data_frame_t {
    decoded_frame_t dfs[MAX_DATA_FRAMES];
    int fn[MAX_DATA_FRAMES];
    bool crc_ok[MAX_DATA_FRAMES];
    int nframes;
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

void data_frame_reset(data_frame_t *data_fr)
{
    data_fr->nframes = 0;
    data_fr->nerrs = 0;
}

static bool check_parity(data_frame_t *data_fr)
{
    for (int i = 1; i < 1 + 64 + 2; ++i) {
        int parity = 0;
        for (int fr_no = 0; fr_no < data_fr->nframes; ++fr_no) {
            parity ^= data_fr->dfs[fr_no].data[i];
        }
        if (parity) {
            return false;
        }
    }

    return true;
}

static void fix_by_parity(data_frame_t *data_fr)
{
    int err_fr_no = 0;

    for (int fr_no = 0; fr_no < data_fr->nframes; ++fr_no) {
        if (!data_fr->crc_ok[fr_no]) {
            err_fr_no = fr_no;
            break;
        }
    }

    // do not fix parity frame
    if (err_fr_no == data_fr->nframes - 1) {
        return;
    }

    for (int i = 1; i < 1 + 64 + 2; ++i) {
        int bit = 0;
        for (int fr_no = 0; fr_no < data_fr->nframes; ++fr_no) {
            if (fr_no != err_fr_no) {
                bit ^= data_fr->dfs[fr_no].data[i];
            }
            data_fr->dfs[err_fr_no].data[i] = bit;
        }
    }
}

static bool data_frame_check_multiframe(data_frame_t *data_fr)
{
    if (data_fr->nerrs) {
        fix_by_parity(data_fr);
    } else {
        if (!check_parity(data_fr)) {
            printf("MB parity error %d\n", data_fr->nframes);
            data_frame_reset(data_fr);
            return false;
        }
    }

    return true;
}

bool data_frame_push_decoded_frame(data_frame_t *data_fr, decoded_frame_t *df)
{
    if (data_fr->nframes == ARRAY_LEN(data_fr->dfs)) {
        data_frame_reset(data_fr);
    }

    const bool crc_ok = decoded_frame_check_crc(df, FRAME_TYPE_DATA);
    data_fr->nerrs += crc_ok ? 0 : 1;
    data_fr->crc_ok[data_fr->nframes] = crc_ok;

    if (data_fr->nerrs > 1) {
        data_frame_reset(data_fr);
        return false;
    }

    const int fn = df->data[1] | (df->data[2] << 1);
    data_fr->fn[data_fr->nframes] = fn;

    memcpy(&data_fr->dfs[data_fr->nframes], df, sizeof(decoded_frame_t));
    ++data_fr->nframes;

    // single frame
    if (data_fr->nframes == 1) {
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

    const int fn_prev = data_fr->fn[data_fr->nframes - 2];
    const bool crc_ok_prev = data_fr->crc_ok[data_fr->nframes - 2];

    // check for dualframe or multiframe
    if (data_fr->nframes == 2) {
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
            return data_frame_push_decoded_frame(data_fr, df);
        }
        return false;
    }

    // check multiframe, inner frames
    if (data_fr->nframes == 3) {
        if (!crc_ok) {
            return false;
        }
        if (fn != FN_10 && fn != FN_11) {
            data_frame_reset(data_fr);
            return data_frame_push_decoded_frame(data_fr, df);
        }
        return false;
    }

    // end of multiframe, final frame is invalid
    if (!crc_ok) {
        if (fn_prev == FN_10) {
            return data_frame_check_multiframe(data_fr);
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
        return data_frame_check_multiframe(data_fr);
    }

    printf("MB err\n");
    data_frame_reset(data_fr);
    return data_frame_push_decoded_frame(data_fr, df);
}

int data_frame_get_tpdu_data(data_frame_t *data_fr, uint8_t *tpdu_data)
{
    const int nframes = (data_fr->nframes <= 2) ?
        data_fr->nframes : data_fr->nframes - 1;

    printf("MB%d\n", nframes);

    for (int fr_no = 0; fr_no < nframes; ++fr_no) {
        memcpy(tpdu_data, data_fr->dfs[fr_no].data + 3, 64);
        tpdu_data += 64;
    }

    data_frame_reset(data_fr);

    printf("tpdu_data=");
    print_buf(tpdu_data - nframes * 64, nframes * 64);

    return nframes * 64;
}

