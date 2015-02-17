#include <stdio.h>

#include <tetrapol/misc.h>

void print_hex(const uint8_t *bytes, int n)
{
    for(int i = 0; i < n; i++) {
        printf("%02x ", bytes[i]);
        if (i % 8 == 7) {
            printf(" ");
        }
    }
    printf("\n");
}
