#include "sdch.h"
#include "data_frame.h"
#include "system_config.h"

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
    const int size = data_frame_get_bytes(sdch->data_fr, data);

    // TODO ...

    return true;
}

tsdu_t *sdch_get_tsdu(sdch_t *sdch)
{
    // TODO ...
    return NULL;
}
