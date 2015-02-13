#include <stdio.h>

#include "tetrapol/misc.h"


int bits_to_int(const uint8_t *bits, int num)
{
    int ret = 0;

    for (int i = 0; i < num; i++) {
        ret |= (bits[i] << (num - i - 1));
    }

    return ret;
}

void decode_addr(const uint8_t *t)
{
    int x,y,z;

    z = bits_to_int(t, 1);
    y = bits_to_int(t+1, 3);
    x = bits_to_int(t+4, 12);

    if ((z==1) && (y==0))
        printf("RTI:%03x\n", x);
    if ((z==0) && (y==0))
        printf("CGI:%03x\n", x);
    if ((z==0) && (y!=0) && (y!=1)) {
        printf("TTI:%1x%03x",y, x);
        if ((y==7) && (x==0))
            printf(" no ST");
        if ((y==7) && (x==4095))
            printf(" all STs");
        printf("\n");
    }
    if (y==1)
        printf("COI:%03x\n", x);
}

void print_buf(const uint8_t *data, int len)
{
    for(int i = 0; i < len; i++) {
        printf("%x", data[i]);
    }
    printf("\n");
}

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

