#pragma once

void radio_init();

void segmentation_reset(void);
void mod_set(int m);

void print_buf(const uint8_t *frame, int framelen);

