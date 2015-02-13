#pragma once

#include <tetrapol/data_block.h>
#include <tetrapol/tsdu.h>

#include <stdbool.h>

typedef struct _sdch_t sdch_t;

sdch_t *sdch_create(void);
void sdch_destroy(sdch_t *sdch);
bool sdch_dl_push_data_frame(sdch_t *sdch, data_block_t *data_blk);
tsdu_t *sdch_get_tsdu(sdch_t *sdch);
