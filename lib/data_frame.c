#include "data_frame.h"
#include "misc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// maximal amount of data frames including single XOR frame
#define MAX_DATA_FRAMES 9

struct _data_frame_t {
    decoded_frame_t dfs[MAX_DATA_FRAMES];
    int fn[MAX_DATA_FRAMES];
    int nframes;
    int errs;
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
    data_fr->errs = 0;
}

enum {
    FN_00 = 00,
    FN_01 = 01,
    FN_10 = 02,
    FN_11 = 03,
};

bool data_frame_push_decoded_frame(data_frame_t *data_fr, decoded_frame_t *df)
{
    if (data_fr->nframes == ARRAY_LEN(data_fr->dfs)) {
        data_frame_reset(data_fr);
    }

    memcpy(&data_fr->dfs[data_fr->nframes], df, sizeof(decoded_frame_t));
    ++data_fr->nframes;

    const bool crc_ok = decoded_frame_check_crc(df, FRAME_TYPE_DATA);
    if (!crc_ok) {
        data_frame_reset(data_fr);
        // TODO: more inteligent error handling, try repair frames and multiframes
        return false;
    }

    const int fn = df->data[1] | (df->data[2] << 1);
    data_fr->fn[data_fr->nframes - 1] = fn;

    // single frame
    if (data_fr->nframes == 1) {
        if (fn == FN_00) {
            printf("MB1 frame_no=%d\n", df->frame_no);
            return true;
        }
        if (fn != FN_01) {
            printf("mb err\n");
            data_frame_reset(data_fr);
        }
        return false;
    }

    // check for dualframe or multiframe
    if (data_fr->nframes == 2) {
        if (fn == FN_11) {
            printf("MB2 frame_no=%d\n", df->frame_no);
            return true;
        }
        if (fn != FN_10) {
            printf("mb err\n");
            data_frame_reset(data_fr);
            return data_frame_push_decoded_frame(data_fr, df);
        }
        return false;
    }

    // check multiframe, inner frames
    const int fn_prev = data_fr->fn[data_fr->nframes - 2];
    if (fn == FN_11 && (fn_prev == FN_10 || fn_prev == FN_11)) {
        return false;
    }

    // check multiframe, pre-end of multiframe
    if (fn == FN_10 && (fn_prev == FN_10 || fn_prev == FN_11)) {
        return false;
    }

    // check multiframe, end of multiframe
    if (fn == FN_01 && fn_prev == FN_10) {
        // check XOR
        for (int i = 1; i < 1 + 64 + 2; ++i) {
            int r = 0;
            for (int n = 0; n < data_fr->nframes; ++n) {
                r ^= data_fr->dfs[n].data[i];
            }
            if (r) {
                printf("mb xor err %d frame_no=%d\n", data_fr->nframes, df->frame_no);
                data_frame_reset(data_fr);
                return false;
            }
        }
        printf("MB%d frame_no=%d\n", data_fr->nframes - 1, df->frame_no);
        return true;
    }

    printf("mb err\n");
    data_frame_reset(data_fr);
    return data_frame_push_decoded_frame(data_fr, df);
}

int data_frame_get_tpdu_data(data_frame_t *data_fr, uint8_t *tpdu_data)
{
    if (data_fr->nframes == 0) {
        return 0;
    }

    const int nframes = (data_fr->nframes <= 2) ?
        data_fr->nframes : data_fr->nframes - 1;
    for (int i = 0; i < nframes; ++i) {
        memcpy(tpdu_data, data_fr->dfs[i].data + 3, 64);
        tpdu_data += 64;
    }
    if (nframes >= 3) {
        // TODO: check parity, fix broken bites
    }

    data_frame_reset(data_fr);

    printf("tpdu_data=");
    print_buf(tpdu_data-nframes * 64, nframes * 64);

    return nframes * 64;
}

