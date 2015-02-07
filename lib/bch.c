#include "bch.h"

#include <stdbool.h>
#include <stdlib.h>


struct _bch_t {
};

bch_t *bch_create(void)
{
    return malloc(sizeof(bch_t));
}

void bch_destroy(bch_t *bch)
{
    free(bch);
}

bool bch_push_data_block(bch_t *bch, data_block_t* data_blk)
{
    return false;
}

tsdu_system_info_t *bch_get_tsdu(bch_t *bch)
{
    return NULL;
}
