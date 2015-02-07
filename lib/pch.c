#include "pch.h"

#include <stdlib.h>

struct _pch_t {
    data_frame_t *data_fr;
};

pch_t *pch_create(void)
{
    pch_t *pch = malloc(sizeof(pch_t));
    if (!pch) {
        return NULL;
    }

    pch->data_fr = data_frame_create();
    if (!pch->data_fr) {
        free(pch);
        return NULL;
    }

    return pch;
}

void pch_destroy(pch_t *pch)
{
    if (pch) {
        data_frame_destroy(pch->data_fr);
    }
    free(pch);
}

bool pch_push_data_block(pch_t *pch, data_block_t* data_blk)
{
    return false;
}

