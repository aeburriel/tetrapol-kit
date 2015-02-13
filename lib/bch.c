#define LOG_PREFIX "bch"
#include "log.h"
#include "bch.h"
#include "data_frame.h"
#include "hdlc_frame.h"
#include "misc.h"
#include "tpdu.h"
#include "system_config.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct _bch_t {
    data_frame_t *data_fr;
    tpdu_ui_t *tpdu;
    tsdu_d_system_info_t *tsdu;
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

    bch->tpdu = tpdu_ui_create(FRAME_TYPE_DATA);
    if (!bch->tpdu) {
        free(bch);
        data_frame_destroy(bch->data_fr);
        return NULL;
    }

    bch->tsdu = NULL;

    return bch;
}

void bch_destroy(bch_t *bch)
{
    tsdu_destroy(&bch->tsdu->base);
    data_frame_destroy(bch->data_fr);
    tpdu_ui_destroy(bch->tpdu);
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
            IF_LOG(DBG) {
                LOG_("invalid address for BCH");
                addr_print(&hdlc_fr.addr);
                printf("\n");
            }
        }
        return false;
    }

    tpdu_ui_push_hdlc_frame2(bch->tpdu, &hdlc_fr);
    if (!tpdu_ui_has_tsdu(bch->tpdu)) {
        return false;
    }

    tsdu_t *tsdu = tpdu_ui_get_tsdu(bch->tpdu);
    if (tsdu->codop != D_SYSTEM_INFO) {
        LOG(DBG, "Invalid codop for BCH 0x%02x", tsdu->codop);
        tsdu_destroy(tsdu);

        return false;
    }

    tsdu_destroy(&bch->tsdu->base);
    bch->tsdu = (tsdu_d_system_info_t *)tsdu;

    const int frame_no = 100 * bch->tsdu->cell_state.bch + nblocks - 1;
    if (data_blk->frame_no != FRAME_NO_UNKNOWN &&
            frame_no != data_blk->frame_no) {
        LOG(ERR, "Frame skew detected %d to %d\n",
                data_blk->frame_no, frame_no);
    }
    data_blk->frame_no = frame_no;

    return true;
}

tsdu_d_system_info_t *bch_get_tsdu(bch_t *bch)
{
    tsdu_d_system_info_t *tsdu = bch->tsdu;
    bch->tsdu = NULL;

    return tsdu;
}
