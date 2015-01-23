#pragma once

#include <stdint.h>

int bits_to_int(const uint8_t *bits, int num);
void decode_addr(const uint8_t *t);
void print_buf(const uint8_t *frame, int framelen);

