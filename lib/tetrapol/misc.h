#pragma once

#include <stdint.h>

#define ARRAY_LEN(a) (sizeof(a) / sizeof(a[0]))

// TODO: transfer to log.h
void print_hex(const uint8_t *bytes, int n);
