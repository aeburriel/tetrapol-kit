#define LOG_PREFIX "sdch"
#include <tetrapol/log.h>
#include <tetrapol/sdch.h>
#include <tetrapol/data_frame.h>
#include <tetrapol/hdlc_frame.h>
#include <tetrapol/misc.h>
#include <tetrapol/tpdu.h>
#include <tetrapol/system_config.h>

#include <stdlib.h>
#include <string.h>

struct _sdch_t {
    data_frame_t *data_fr;
    tpdu_t *tpdu;
    tpdu_ui_t *tpdu_ui;
};

sdch_t *sdch_create(void)
{
    sdch_t *sdch = malloc(sizeof(sdch_t));
    if (!sdch) {
        return NULL;
    }

    sdch->data_fr = data_frame_create();
    if (!sdch->data_fr) {
        goto err_data_fr;
    }

    sdch->tpdu_ui = tpdu_ui_create(FRAME_TYPE_DATA);
    if (!sdch->tpdu_ui) {
        goto err_tpdu_ui;
    }

    sdch->tpdu = tpdu_create();
    if (!sdch->tpdu) {
        goto err_tpdu;
    }

    return sdch;

err_tpdu:
    tpdu_ui_destroy(sdch->tpdu_ui);

err_tpdu_ui:
    data_frame_destroy(sdch->data_fr);

err_data_fr:
    free(sdch);

    return NULL;
}

void sdch_destroy(sdch_t *sdch)
{
    if (sdch) {
        data_frame_destroy(sdch->data_fr);
        tpdu_ui_destroy(sdch->tpdu_ui);
        tpdu_destroy(sdch->tpdu);
    }
    free(sdch);
}

bool sdch_dl_push_data_frame(sdch_t *sdch, data_block_t *data_blk)
{
    if (!data_frame_push_data_block(sdch->data_fr, data_blk)) {
        return false;
    }

    uint8_t data[SYS_PAR_N200_BYTES_MAX];
    const int size = data_frame_get_bytes(sdch->data_fr, data);

    hdlc_frame_t hdlc_fr;

    if (!hdlc_frame_parse(&hdlc_fr, data, size)) {
        // PAS 0001-3-3 7.4.1.9 stuffing frames are dropped, FCS does not match
        return false;
    }

    if (hdlc_fr.command.cmd == COMMAND_INFORMATION ||
            hdlc_fr.command.cmd == COMMAND_SUPERVISION_RR ||
            hdlc_fr.command.cmd == COMMAND_SUPERVISION_RNR ||
            hdlc_fr.command.cmd == COMMAND_SUPERVISION_REJ) {
        return tpdu_push_hdlc_frame(sdch->tpdu, &hdlc_fr);
    }

    if (hdlc_fr.command.cmd == COMMAND_UNNUMBERED_UI) {
        IF_LOG(DBG) {
            LOG_("HDLC info=");
            print_hex(hdlc_fr.data, hdlc_fr.nbits / 8);
            printf("\t");
            addr_print(&hdlc_fr.addr);
            printf("\n");
        }
        return tpdu_ui_push_hdlc_frame(sdch->tpdu_ui, &hdlc_fr);
    }

    if (hdlc_fr.command.cmd == COMMAND_DACH) {
        IF_LOG(INFO) {
            LOG_("\n\tcmd ACK_DACH\n\taddr: ");
            addr_print(&hdlc_fr.addr);
            printf("\n");
        }
        if (!cmpzero(hdlc_fr.data, hdlc_fr.nbits / 8)) {
            IF_LOG(WTF) {
                LOG_("cmd: ACK_DACH, nonzero stuffing");
                print_hex(hdlc_fr.data, hdlc_fr.nbits / 8);
            }
        }

        LOG(ERR, "TODO: ACK_DACH");
        // TODO: report ACK_DACH to application layer
        return false;
    }

    if (hdlc_fr.command.cmd == COMMAND_UNNUMBERED_SNRM) {
        IF_LOG(INFO) {
            LOG_("\n\tcmd SNMR\n\taddr: ");
            addr_print(&hdlc_fr.addr);
            printf("\n");
        }

        if (!cmpzero(hdlc_fr.data, hdlc_fr.nbits / 8)) {
            IF_LOG(WTF) {
                LOG_("cmd: SNMR, nonzero stuffing");
                print_hex(hdlc_fr.data, hdlc_fr.nbits / 8);
            }
        }

        LOG(ERR, "TODO: SNMR");
        // TODO: report SNMR to upper layer
        return false;
    }

    LOG(INFO, "TODO CMD 0x%02x", hdlc_fr.command.cmd);

    return false;
}

tsdu_t *sdch_get_tsdu(sdch_t *sdch)
{
    tsdu_t *tsdu = tpdu_ui_get_tsdu(sdch->tpdu_ui);

    // TODO: multiplexing for other TPDU types

    return tsdu;
}

void sdch_tick(const timeval_t *tv, void *sdch)
{
    tpdu_du_tick(tv, ((sdch_t *)sdch)->tpdu_ui);
}
