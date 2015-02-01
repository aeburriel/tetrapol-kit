#pragma once

#include "decoded_frame.h"

void multiblock_reset(void);
void multiblock_process(decoded_frame_t *df, int fn);
