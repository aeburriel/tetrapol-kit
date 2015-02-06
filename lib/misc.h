#pragma once

#include <stdint.h>

#define ARRAY_LEN(a) (sizeof(a) / sizeof(a[0]))

int bits_to_int(const uint8_t *bits, int num);
void decode_addr(const uint8_t *t);
void print_buf(const uint8_t *frame, int framelen);

void print_hex(const uint8_t *bytes, int n);
