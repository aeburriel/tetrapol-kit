#include "bch.h"
#include "data_frame.h"
#include "hdlc_frame.h"
#include "misc.h"
#include "system_config.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct _bch_t {
    data_frame_t *data_fr;
    tsdu_system_info_t *tsdu;
};

bch_t *bch_create(void)
{
    bch_t *bch = malloc(sizeof(bch_t));
    if (!bch) {
        return NULL;
    }

    bch->data_fr = data_frame_create();
    if (!bch->data_fr) {
        free(bch);
        return NULL;
    }

    bch->tsdu = NULL;

    return bch;
}

void bch_destroy(bch_t *bch)
{
    tsdu_destroy(&bch->tsdu->base);
    data_frame_destroy(bch->data_fr);
    free(bch);
}

bool bch_push_data_block(bch_t *bch, data_block_t* data_blk)
{
    if (!data_frame_push_data_block(bch->data_fr, data_blk)) {
        return false;
    }

    uint8_t tpdu_data[SYS_PAR_N200_BYTES_MAX];
    const int nblocks = data_frame_blocks(bch->data_fr);
    const int size = data_frame_get_bytes(bch->data_fr, tpdu_data);

    hdlc_frame_t hdlc_fr;
    if (!hdlc_frame_parse(&hdlc_fr, tpdu_data, size)) {
        return false;
    }

    if (hdlc_fr.command.cmd != COMMAND_UNNUMBERED_UI) {
        return false;
    }

    if (!addr_is_tti_all_st(&hdlc_fr.addr, true)) {
        if (data_blk->frame_no == FRAME_NO_UNKNOWN) {
            printf("invalid address for BCH ");
            addr_print(&hdlc_fr.addr);
            printf("\n");
        }
        return false;
    }

    tsdu_destroy(&bch->tsdu->base);
    bch->tsdu = (tsdu_system_info_t *)tsdu_decode(
            hdlc_fr.info+2, hdlc_fr.info_nbits - 16);
    if (bch->tsdu == NULL) {
        return false;
    }

    if (bch->tsdu->base.codop != D_SYSTEM_INFO) {
        if (data_blk->frame_no == FRAME_NO_UNKNOWN) {
            printf("Invalid codop for BCH %d\n", bch->tsdu->base.codop);
        }
        tsdu_destroy(&bch->tsdu->base);
        bch->tsdu = NULL;

        return false;
    }

    printf("\tBCH\n");
    printf("\tRT_REF=TODO\n");
    printf("\tBS_REF=TODO\n");
    printf("\tCALL_PRIO=TODO\n");
    // TODO: add proper TPDU layer handler

    const int frame_no = 100 * bch->tsdu->cell_state.bch + nblocks - 1;
    if (data_blk->frame_no != FRAME_NO_UNKNOWN &&
            frame_no != data_blk->frame_no) {
        printf("Frame skew detected %d to %d\n",
                data_blk->frame_no, frame_no);
    }
    data_blk->frame_no = frame_no;

    return true;
}

tsdu_system_info_t *bch_get_tsdu(bch_t *bch)
{
    tsdu_system_info_t *tsdu = bch->tsdu;
    bch->tsdu = NULL;

    return tsdu;
}
