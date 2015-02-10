#pragma once

#include "data_block.h"
#include "tsdu.h"

typedef struct _bch_t bch_t;

bch_t *bch_create(void);
void bch_destroy(bch_t *bch);
bool bch_push_data_block(bch_t *bch, data_block_t* data_blk);
tsdu_d_system_info_t *bch_get_tsdu(bch_t *bch);
