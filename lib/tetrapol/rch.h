#pragma once

#include <tetrapol/data_block.h>

#include <stdbool.h>

typedef struct _rch_t rch_t;

rch_t *rch_create(void);
void rch_destroy(rch_t *rch);
bool rch_push_data_block(rch_t *rch, data_block_t *data_blk);
void rch_print(const rch_t *rch);

