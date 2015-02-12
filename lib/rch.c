#include "rch.h"
#include "addr.h"
#include "bit_utils.h"
#include "data_frame.h"
#include "misc.h"
#include "system_config.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int naddrs;
    addr_t addrs[3];
} rch_data_t;

struct _rch_t {
     data_frame_t *data_fr;
     rch_data_t rch_data;
};

rch_t *rch_create(void)
{
    rch_t *rch = malloc(sizeof(rch_t));
    if (!rch) {
        return NULL;
    }

    rch->data_fr = data_frame_create();
    if (!rch->data_fr) {
        free(rch);
        return NULL;
    }

    return rch;
}

void rch_destroy(rch_t *rch)
{
    if (rch) {
        data_frame_destroy(rch->data_fr);
    }
    free(rch);
}

bool rch_push_data_block(rch_t *rch, data_block_t *data_blk)
{
    if (!data_frame_push_data_block(rch->data_fr, data_blk)) {
        printf("RCH: block fail\n");
        return false;
    }

    if (data_frame_blocks(rch->data_fr) != 1) {
        printf("RCH: WTF block lenth != 1\n");
        return false;
    }

    uint8_t data[(92 + 7) / 8];
    const int size = data_frame_get_bytes(rch->data_fr, data);
    if (size != 64) {
        printf("RCH: WTF invalid frame lenght\n");
        return false;
    }

    if (!check_fcs(data, size)) {
        printf("RCH: invalid FCS\n");
        return false;
    }

    rch->rch_data.naddrs = 0;
    for (int i = 0; i < ARRAY_LEN(rch->rch_data.addrs); ++i) {
        addr_parse(&rch->rch_data.addrs[rch->rch_data.naddrs], data + 2*i, 0);
        if (!addr_is_tti_no_st(&rch->rch_data.addrs[rch->rch_data.naddrs], true)) {
            ++rch->rch_data.naddrs;
        }
    }

    return true;
}

void rch_print(const rch_t *rch)
{
    printf("RCH ACKs (%d):\n", rch->rch_data.naddrs);
    for (int i = 0; i < rch->rch_data.naddrs; ++i) {
        const addr_t *addr = &rch->rch_data.addrs[i];
        if (!addr->z) {
            printf("\tADDR ACK: ");
            addr_print(addr);
            printf("\n");
        } else {
            printf("\tNACK: ");
            if (addr->y == 4) {
                printf("noise\n");
            } else if (addr->y == 5) {
                printf("collision\n");
            } else {
                printf("WTF unknown\n");
            }
        }
    }
}
