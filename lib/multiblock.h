#pragma once

#include "decoded_frame.h"

void multiblock_reset(void);
void multiblock_process(data_block_t *data_blk, int fn);
