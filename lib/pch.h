#pragma once

#include <stdbool.h>

#include "data_frame.h"

typedef struct _pch_t pch_t;

pch_t *pch_create(void);
void pch_destroy(pch_t *pch);

/**
  Should be called when some frames are missing.
  */
void pch_reset(pch_t *pch);
bool pch_push_data_block(pch_t *pch, data_block_t* data_blk);
void pch_print(pch_t *pch);
