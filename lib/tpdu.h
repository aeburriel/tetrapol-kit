#pragma once

#include <stdint.h>

void segmentation_reset(void);
void tpdu_process(const uint8_t *t, int length, int *frame_no);

