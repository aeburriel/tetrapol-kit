#include "sdch.h"
#include "data_frame.h"
#include "misc.h"
#include "tpdu.h"
#include "system_config.h"

#include <stdio.h>
#include <stdlib.h>

struct _sdch_t {
    data_frame_t *data_fr;
};

sdch_t *sdch_create(void)
{
    sdch_t *sdch = malloc(sizeof(sdch_t));
    if (!sdch) {
        return NULL;
    }
    sdch->data_fr = data_frame_create();
    if (!sdch->data_fr) {
        free(sdch);
        return NULL;
    }

    return sdch;
}

void sdch_destroy(sdch_t *sdch)
{
    if (sdch) {
        data_frame_destroy(sdch->data_fr);
    }
    free(sdch);
}

bool sdch_dl_push_data_frame(sdch_t *sdch, data_block_t *data_blk)
{
    if (!data_frame_push_data_block(sdch->data_fr, data_blk)) {
        return false;
    }

    uint8_t data[SYS_PAR_N200_BYTES_MAX];
    int nblks = data_frame_blocks(sdch->data_fr);
    const int size = data_frame_get_bytes(sdch->data_fr, data);

    // TODO ...


    // TODO: replace - this is just hack for old decoding methods
    // unpack into old format with swapped bite order
    uint8_t data_[SYS_PAR_N200_BYTES_MAX * 8];
    for (int i = size - 1; i > 0; ) {
        data_[i - 0] = (data[i / 8] >> 0) & 1;
        data_[i - 1] = (data[i / 8] >> 1) & 1;
        data_[i - 2] = (data[i / 8] >> 2) & 1;
        data_[i - 3] = (data[i / 8] >> 3) & 1;
        data_[i - 4] = (data[i / 8] >> 4) & 1;
        data_[i - 5] = (data[i / 8] >> 5) & 1;
        data_[i - 6] = (data[i / 8] >> 6) & 1;
        data_[i - 7] = (data[i / 8] >> 7) & 1;
        i -= 8;
    }

    int frame_no = data_blk->frame_no - nblks + 1;
    nblks = nblks > 2 ? nblks - 1 : nblks;
    printf("MB%d frame_no=%03d ", nblks, frame_no);
    print_buf(data_, size);
    tpdu_process(data_, size / 8, &frame_no);

    return true;
}

tsdu_t *sdch_get_tsdu(sdch_t *sdch)
{
    // TODO ...
    return NULL;
}
