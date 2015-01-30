#include <string.h>
#include <stdio.h>
#include "tpdu.h"
#include "tsdu.h"
#include "misc.h"
#include "multiblock.h"


int state, numblocks, startmod;
uint8_t buf[8*8*9];

void multiblock_reset(void) {

    state=0;
    numblocks=0;
}

int multiblock_xor_verify(uint8_t *frame, int num) {

    int i, j, acc;

    for (i=0; i<64; i++) {
        acc=0;
        for (j=0; j<num; j++)
            acc = acc ^ frame[j*64 + i];
        if (acc)
            return 0;
    }
    return 1;
}

#define FRAME_BITORDER_LEN 64

static void bitorder_frame(const uint8_t *d, uint8_t *out)
{
    for (int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            out[8*i + j] =  d[i*8 + 7-j];
        }
    }
}

void multiblock_process(data_frame_t *df, int fn, int mod)
{
    uint8_t frame_bord[FRAME_BITORDER_LEN];
    bitorder_frame(df->data + 3, frame_bord);

    if (mod%25 == 14) {			// FIXME: RCH, PCH ??
        decode_rch(frame_bord);
        return;
    }

    switch(state) {
        case 0:
            numblocks=0;
            startmod=mod;
            switch(fn) {
                case 0:
                    memcpy(buf, frame_bord, 64);
                    printf("MB1 mod=%03i ", startmod);
                    print_buf(buf, 64);
                    tpdu_process(buf, 8, startmod);
                    state=0;
                    break;
                case 1:
                    memcpy(buf+64*numblocks, frame_bord, 64);
                    numblocks++;
                    state=1;
                    break;
                case 2:
                    printf("mb err\n");
                    state=0;
                    segmentation_reset();
                    break;
                case 3:
                    printf("mb err\n");
                    state=0;
                    segmentation_reset();
                    break;
            }
            break;
        case 1:
            switch(fn) {
                case 0:
                    printf("mb err\n");
                    state=0;
                    segmentation_reset();
                    break;
                case 1:
                    printf("mb err\n");
                    state=0;
                    segmentation_reset();
                    break;
                case 2:
                    memcpy(buf+64*numblocks, frame_bord, 64);
                    numblocks++;
                    state=2;
                    break;
                case 3:
                    memcpy(buf+64*numblocks, frame_bord, 64);
                    printf("MB2 mod=%03i ", startmod);
                    print_buf(buf, 128);
                    tpdu_process(buf, 16, startmod);
                    state=0;
                    break;
            }
            break;
        case 2:
            switch(fn) {
                case 0:
                    printf("mb err\n");
                    state=0;
                    break;
                case 1:
                    printf("mb err\n");
                    state=0;
                    break;
                case 2:
                    memcpy(buf+64*numblocks, frame_bord, 64);
                    numblocks++;
                    state=3;
                    break;
                case 3:
                    memcpy(buf+64*numblocks, frame_bord, 64);
                    numblocks++;
                    state=4;
                    break;
            }
            break;
        case 3:
            switch(fn) {
                case 0:
                    printf("mb err\n");
                    state=0;
                    segmentation_reset();
                    break;
                case 1:
                    memcpy(buf+64*numblocks, frame_bord, 64);
                    numblocks++;
                    if (multiblock_xor_verify(buf, numblocks)) {
                        printf("MB%i mod=%03i ", numblocks-1, startmod);
                        print_buf(buf, (numblocks-1) * 64);
                        tpdu_process(buf, (numblocks-1)*8, startmod);
                    } else {
                        printf("mb xor err %i mod=%03i ", numblocks, startmod);
                        print_buf(buf, (numblocks-1) * 64);
                    }
                    state=0;
                    break;

                case 2:
                    printf("mb err\n");
                    state=0;
                    segmentation_reset();
                    break;
                case 3:
                    printf("mb err\n");
                    state=0;
                    segmentation_reset();
                    break;
            }
            break;
        case 4:
            switch(fn) {
                case 0:
                    printf("mb err\n");
                    state=0;
                    segmentation_reset();
                    break;
                case 1:
                    printf("mb err\n");
                    state=0;
                    segmentation_reset();
                    break;
                case 2:
                    memcpy(buf+64*numblocks, frame_bord, 64);
                    numblocks++;
                    state=3;
                    break;
                case 3:
                    memcpy(buf+64*numblocks, frame_bord, 64);
                    numblocks++;
                    state=4;
                    break;
            }
            break;
    }
}
