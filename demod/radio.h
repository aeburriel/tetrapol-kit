#pragma once

void radio_init();

void radio_process_frame(uint8_t *frame, int framelen, int modulo);
void segmentation_reset(void);
void mod_set(int m);

void print_buf(uint8_t *frame, int framelen);
void mk_crc5(uint8_t *res, const uint8_t *input, int input_len);

