#define LOG_PREFIX "tpdu"
#include <tetrapol/log.h>
#include <tetrapol/misc.h>
#include <tetrapol/tsdu.h>
#include <tetrapol/tpdu.h>
#include <tetrapol/misc.h>

#include <stdlib.h>
#include <string.h>

enum {
    TPDU_CODE_CR = 0,
    TPDU_CODE_CC = 0x8,
    TPDU_CODE_FCR = 0x10,
    TPDU_CODE_DR = 0x18,
    TPDU_CODE_FDR = 0x19,
    TPDU_CODE_DC = 0x20,
    TPDU_CODE_DT = 0x21,
    TPDU_CODE_DTE = 0x22,
};

#define TPDU_CODE_PREFIX_MASK (0x18)

typedef struct {
    timeval_t tv;
    uint8_t id_tsap;
    uint8_t prio;
    uint8_t nsegments;  ///< total amount of segments (HDLC frames) in DU
    hdlc_frame_t *hdlc_frs[SYS_PAR_N452];
} segmented_du_t;

typedef struct {
    uint8_t tsap_id;
    uint8_t tsap_ref_dl;
    uint8_t tsap_ref_ul;
} connection_t;

struct _tpdu_t {
    connection_t *conns[16];  // listed by TSAP reference id (SwMI side)
    connection_t *conns_fast[16];  // listed by TSAP id
};

struct _tpdu_ui_t {
    frame_type_t fr_type;
    segmented_du_t *seg_du[128];
    tsdu_t *tsdu;           ///< contains last decoded TSDU
};

tpdu_t *tpdu_create(void)
{
    tpdu_t *tpdu = calloc(1, sizeof(tpdu_t));
    if (!tpdu) {
        return NULL;
    }

    return tpdu;
}

static bool tpdu_push_supervision_frame(tpdu_t *tpdu, const hdlc_frame_t *hdlc_fr)
{
    IF_LOG(INFO) {
        switch(hdlc_fr->command.cmd) {
            case COMMAND_SUPERVISION_RR:
                LOG_("\n\tcmd: RR\n\taddr: ");
                break;

            case COMMAND_SUPERVISION_RNR:
                LOG_("\n\tcmd: RNR\n\taddr: ");
                break;

            case COMMAND_SUPERVISION_REJ:
                LOG_("\n\tcmd: REJ\n\taddr: ");
                break;
        }
        addr_print(&hdlc_fr->addr);
        printf("\n\trecv_seq_no: %d P: %d\n",
               hdlc_fr->command.supervision.recv_seq_no,
               hdlc_fr->command.supervision.p_e);
    }

    if (!cmpzero(hdlc_fr->data, hdlc_fr->nbits / 8)) {
        IF_LOG(WTF) {
            LOG_("cmd: 0x%02x, nonzero stuffing", hdlc_fr->command.cmd);
            print_hex(hdlc_fr->data, hdlc_fr->nbits / 8);
        }
    }

    switch(hdlc_fr->command.cmd) {
        case COMMAND_SUPERVISION_RR:
            // TODO: do something with TPDU
            LOG(ERR, "TODO RR");
            break;

        case COMMAND_SUPERVISION_RNR:
            // TODO: do something with TPDU
            LOG(ERR, "TODO RNR");
            break;

        case COMMAND_SUPERVISION_REJ:
            // TODO: do something with TPDU
            LOG(ERR, "TODO REJ");
            break;
    }

    return false;
}

static bool tpdu_push_information_frame(tpdu_t *tpdu, const hdlc_frame_t *hdlc_fr)
{
    const bool ext              = get_bits(1, hdlc_fr->data, 0);
    const bool seg              = get_bits(1, hdlc_fr->data, 1);
    const bool d                = get_bits(1, hdlc_fr->data, 2);
    const uint8_t code          = get_bits(5, hdlc_fr->data, 3);
    const uint8_t par_field     = get_bits(4, hdlc_fr->data + 1, 0);
    const uint8_t dest_ref      = get_bits(4, hdlc_fr->data + 1, 4);

    if (ext) {
        LOG(WTF, "ext != 0 for TPDU");
    }

    const uint8_t len = (d && !seg) ? hdlc_fr->data[2] : 0;

    IF_LOG(INFO) {
        LOG_("information cmd\n");
        printf("\taddr: ");
        addr_print(&hdlc_fr->addr);
        printf("\n\trecv_seq_no: %d send_seq_no: %d P: %d\n",
               hdlc_fr->command.information.recv_seq_no,
               hdlc_fr->command.information.send_seq_no,
               hdlc_fr->command.information.p_e);
    }

    const uint8_t code_prefix   = code & TPDU_CODE_PREFIX_MASK;

    if (code_prefix != TPDU_CODE_PREFIX_MASK) {
        const uint8_t qos           = (~TPDU_CODE_PREFIX_MASK) & code;

        switch(code_prefix) {
            case 0: // CR
                LOG(ERR, "TODO CR seg: %d d: %d TSAP_ref: %d TSAP_id %d QoS: %d len: %d",
                    seg, d, par_field, dest_ref, qos, len);
                // check if connection exists, deallocate and WTF
                // create new connection stuct
                // set state to REQ

                break;

            case 1: // CC
                LOG(ERR, "TODO CC seg: %d d: %d TSAP_ref_send: %d TSAP_ref_recv: %d QoS: %d len: %d",
                    seg, d, par_field, dest_ref, qos, len);
                // check if connection exists
                break;

            case 2: //FCR
                LOG(ERR, "TODO FCR seg: %d d: %d TSAP_ref: %d TSAP_id: %d QoS: %d len: %d",
                    seg, d, par_field, dest_ref, qos, len);
                break;
        }
    } else {
        switch (code) {
            case TPDU_CODE_DR:
                LOG(ERR, "TODO DR d: %d TSAP_ref_send: %d TSAP_ref_recv: %d len: %d",
                    d, par_field, dest_ref, len);
                break;

            case TPDU_CODE_FDR:
                LOG(ERR, "TODO FDR d: %d TSAP_ref_send: %d TSAP_ref_recv: %d len: %d",
                    d, par_field, dest_ref, len);
                break;

            case TPDU_CODE_DC:
                LOG(ERR, "TODO DC TSAP_ref_send: %d TSAP_ref_recv: %d",
                    par_field, dest_ref);
                break;

            case TPDU_CODE_DT:
                LOG(ERR, "TODO DT seg: %d d: %d TSAP_ref_send: %d TSAP_ref_recv: %d, len: %d",
                    seg, d, par_field, dest_ref, len);
                break;

            case TPDU_CODE_DTE:
                LOG(ERR, "TODO DTE TSAP_ref_send: %d TSAP_ref_recv: %d, len: %d",
                    par_field, dest_ref, len);
                break;

            default:
                LOG(WTF, "unknown code %d", code);
        }
    }

    return hdlc_fr;
}

bool tpdu_push_hdlc_frame(tpdu_t *tpdu, const hdlc_frame_t *hdlc_fr)
{
    switch (hdlc_fr->command.cmd) {
        case COMMAND_SUPERVISION_RR:
        case COMMAND_SUPERVISION_RNR:
        case COMMAND_SUPERVISION_REJ:
            return tpdu_push_supervision_frame(tpdu, hdlc_fr);

        case COMMAND_INFORMATION:
            return tpdu_push_information_frame(tpdu, hdlc_fr);

        default:
            break;
    }

    LOG(WTF, "invalid cmd for TPDU (%d)", hdlc_fr->command.cmd);
    return false;
}

void tpdu_destroy(tpdu_t *tpdu)
{
    free(tpdu);
}

void tpdu_ui_segments_destroy(segmented_du_t *du)
{
    for (int i = 0; i < SYS_PAR_N452; ++i) {
        free(du->hdlc_frs[i]);
    }
    free(du);
}

tpdu_ui_t *tpdu_ui_create(frame_type_t fr_type)
{
    if (fr_type != FRAME_TYPE_DATA && fr_type != FRAME_TYPE_HR_DATA) {
        LOG(ERR, "usnupported frame type %d", fr_type);
        return NULL;
    }

    tpdu_ui_t *tpdu = calloc(1, sizeof(tpdu_ui_t));
    if (!tpdu) {
        return NULL;
    }
    tpdu->fr_type = fr_type;

    return tpdu;
}

void tpdu_ui_destroy(tpdu_ui_t *tpdu)
{
    tsdu_destroy(tpdu->tsdu);
    for (int i = 0; i < ARRAY_LEN(tpdu->seg_du); ++i) {
        if (!tpdu->seg_du[i]) {
            continue;
        }
        tpdu_ui_segments_destroy(tpdu->seg_du[i]);
    }
    free(tpdu);
}


static bool tpdu_ui_push_hdlc_frame_(tpdu_ui_t *tpdu, const hdlc_frame_t *hdlc_fr,
                                     bool allow_seg)
{
    if (hdlc_fr->nbits < 8) {
        LOG(WTF, "too short HDLC (%d)", hdlc_fr->nbits);
        return false;
    }

    bool ext                    = get_bits(1, hdlc_fr->data, 0);
    const bool seg              = get_bits(1, hdlc_fr->data, 1);
    const uint8_t prio          = get_bits(2, hdlc_fr->data, 2);
    const uint8_t id_tsap       = get_bits(4, hdlc_fr->data, 4);

    LOG(DBG, "DU EXT=%d SEG=%d PRIO=%d ID_TSAP=%d", ext, seg, prio, id_tsap);
    if (ext == 0 && seg == 0) {
        tsdu_destroy(tpdu->tsdu);

        // PAS 0001-3-3 9.5.1.2
        if ((tpdu->fr_type == FRAME_TYPE_DATA && hdlc_fr->nbits > (3*8)) ||
                (tpdu->fr_type == FRAME_TYPE_DATA && hdlc_fr->nbits > (6*8))) {
            const int nbits     = get_bits(8, hdlc_fr->data + 1, 0) * 8;
            tpdu->tsdu = tsdu_d_decode(hdlc_fr->data + 2, nbits, prio, id_tsap);
        } else {
            const int nbits = hdlc_fr->nbits - 8;
            tpdu->tsdu = tsdu_d_decode(hdlc_fr->data + 1, nbits, prio, id_tsap);
        }
        return tpdu->tsdu != NULL;
    }

    if (ext != 1) {
        LOG(WTF, "unsupported ext and seg combination");
        return false;
    }

    if (!allow_seg) {
        return false;
    }

    ext                         = get_bits(1, hdlc_fr->data + 1, 0);
    uint8_t seg_ref             = get_bits(7, hdlc_fr->data + 1, 1);
    if (!ext) {
        LOG(WTF, "unsupported short ext");
        return false;
    }

    ext                         = get_bits(1, hdlc_fr->data + 2, 0);
    const bool res              = get_bits(1, hdlc_fr->data + 2, 1);
    const uint8_t packet_num    = get_bits(6, hdlc_fr->data + 2, 2);
    if (ext) {
        LOG(WTF, "unsupported long ext");
        return false;
    }

    if (res) {
        LOG(WTF, "res != 0");
    }
    LOG(DBG, "UI SEGM_REF=%d, PACKET_NUM=%d", seg_ref, packet_num);

    segmented_du_t *seg_du = tpdu->seg_du[seg_ref];
    if (seg_du == NULL) {
        seg_du = calloc(1, sizeof(segmented_du_t));
        if (!seg_du) {
            return false;
        }
        tpdu->seg_du[seg_ref] = seg_du;
        seg_du->id_tsap = id_tsap;
        seg_du->prio = prio;
    }

    if (seg_du->hdlc_frs[packet_num]) {
        // segment already recieved
        return false;
    }

    seg_du->hdlc_frs[packet_num] = malloc(sizeof(hdlc_frame_t));
    if (!seg_du->hdlc_frs[packet_num]) {
        LOG(ERR, "ERR OOM");
        return false;
    }
    memcpy(seg_du->hdlc_frs[packet_num], hdlc_fr, sizeof(hdlc_frame_t));

    if (seg == 0) {
        seg_du->nsegments = packet_num + 1;
    }

    // reset T454 timer
    seg_du->tv.tv_sec = 0;
    seg_du->tv.tv_usec = 0;

    // last segment is still missing
    if (!seg_du->nsegments) {
        return false;
    }

    // check if we have all segments
    for (int i = 0; i < seg_du->nsegments; ++i) {
        if (!seg_du->hdlc_frs[i]) {
            return false;
        }
    }

    // max_segments * (sizeof(hdlc_frame_t->data) - TPDU_DU_header)
    uint8_t data[SYS_PAR_N452 * (sizeof(((hdlc_frame_t*)NULL)->data) - 3)];
    int nbits = 0;
    // collect data from all segments
    if (tpdu->fr_type == FRAME_TYPE_DATA) {
        uint8_t *d = data;
        for (int i = 0; i < seg_du->nsegments; ++i) {
            hdlc_fr = seg_du->hdlc_frs[i];
            int n_ext = 1;
            // skip ext headers
            while (get_bits(1, hdlc_fr->data + n_ext - 1, 0)) {
                ++n_ext;
            }
            int n;
            if (i == (seg_du->nsegments - 1)) {
                n = hdlc_fr->data[n_ext++] * 8;
            } else {
                n = hdlc_fr->nbits - 8 * n_ext;
            }
            nbits += n;
            memcpy(d, &hdlc_fr->data[n_ext], n / 8);
            d += n / 8;
        }
    } else {    // FRAME_TYPE_HR_DATA
        // TODO
    }

    tpdu_ui_segments_destroy(seg_du);
    tpdu->seg_du[seg_ref] = NULL;

    tpdu->tsdu = tsdu_d_decode(data, nbits, prio, id_tsap);

    return tpdu->tsdu != NULL;
}

bool tpdu_ui_push_hdlc_frame(tpdu_ui_t *tpdu, const hdlc_frame_t *hdlc_fr)
{
    return tpdu_ui_push_hdlc_frame_(tpdu, hdlc_fr, true);
}

bool tpdu_ui_push_hdlc_frame2(tpdu_ui_t *tpdu, const hdlc_frame_t *hdlc_fr)
{
    return tpdu_ui_push_hdlc_frame_(tpdu, hdlc_fr, false);
}

tsdu_t *tpdu_ui_get_tsdu(tpdu_ui_t *tpdu)
{
    tsdu_t *tsdu = tpdu->tsdu;
    tpdu->tsdu = NULL;
    return tsdu;
}

void tpdu_du_tick(const timeval_t *tv, void *tpdu_du)
{
    tpdu_ui_t *tpdu = tpdu_du;

    // set/check T454 timer
    for (int i = 0; i < ARRAY_LEN(tpdu->seg_du); ++i) {
        if (!tpdu->seg_du[i]) {
            continue;
        }

        if (!tpdu->seg_du[i]->tv.tv_sec && !tpdu->seg_du[i]->tv.tv_usec) {
            tpdu->seg_du[i]->tv.tv_sec = tv->tv_sec;
            tpdu->seg_du[i]->tv.tv_usec = tv->tv_usec;
            continue;
        }

        if (timeval_abs_delta(&tpdu->seg_du[i]->tv, tv) < SYS_PAR_T454) {
            continue;
        }

        // TODO: report error to application layer
        tpdu_ui_segments_destroy(tpdu->seg_du[i]);
        tpdu->seg_du[i] = NULL;
    }
}

// ------- old methods

uint8_t segbuf[10000];
int numoctets, startmod;

void segmentation_reset(void) {
    numoctets=0;
}

static void tpdu_du_process(const uint8_t* t, int length, int mod) {

    int ext, seg, prio, id_tsap;
    int data_length=0;
    int segmentation_reference, packet_number;

    ext=bits_to_int(t, 1);
    seg=bits_to_int(t+1, 1);
    prio=bits_to_int(t+2, 2);
    id_tsap=bits_to_int(t+4, 4);

    if (ext==0) {		// No segmentation

        printf("\tDU EXT=%i SEG=%i PRIO=%i ID_TSAP=%i DATA_LENGTH=%i\n", ext, seg, prio, id_tsap, length);

        if (length > 3) {
            data_length=bits_to_int(t+8, 8);
            tsdu_process(t+16, data_length, mod);
        } else
            tsdu_process(t+8, length-1, mod);

    } else {		// Segmentation

        segmentation_reference=bits_to_int(t+9, 7);
        packet_number=bits_to_int(t+18, 6);

        printf("\tDU EXT=%i SEG=%i PRIO=%i ID_TSAP=%i SEGM_REF=%i, PACKET_NUM=%i\n", ext, seg, prio, id_tsap, segmentation_reference, packet_number);

        if (seg==1) {
            memcpy(segbuf+numoctets*8, t+24, (length-3)*8);
            numoctets = numoctets + length-3;
        }
        else {		// Last segment
            data_length=bits_to_int(t+24, 8);
            memcpy(segbuf+numoctets*8, t+32, data_length*8);
            numoctets = numoctets + data_length;
            printf("multiseg %i\n", numoctets);
            print_buf(segbuf, numoctets*8);
            tsdu_process(segbuf, numoctets, startmod);
            numoctets=0;

        }

    }



}

static void tpdu_i_process(const uint8_t* t, int length, int mod) {

    int ext, seg, d, tpdu_code;
    int par_field, dest_ref;
    int data_length;

    ext=bits_to_int(t, 1);
    seg=bits_to_int(t+1, 1);
    d=bits_to_int(t+2, 1);
    tpdu_code=bits_to_int(t+3, 5);

    par_field=bits_to_int(t+8, 4);
    dest_ref=bits_to_int(t+12, 4);

    if ((tpdu_code & 0x18) == 0) {
        printf("\tI CR EXT=%i SEG=%i D=%i TPDU_CODE=%i PAR_FIELD=%i DEST_REF=%i\n", ext, seg, d, tpdu_code, par_field, dest_ref);
    } else if ((tpdu_code & 0x18)  == 8) {
        printf("\tI CC EXT=%i SEG=%i D=%i TPDU_CODE=%i PAR_FIELD=%i DEST_REF=%i\n", ext, seg, d, tpdu_code, par_field, dest_ref);
    } else if ((tpdu_code & 0x18) == 16) {
        printf("\tI FCR EXT=%i SEG=%i D=%i TPDU_CODE=%i PAR_FIELD=%i DEST_REF=%i\n", ext, seg, d, tpdu_code, par_field, dest_ref);
    } else if (tpdu_code == 24) {
        printf("\tI DR EXT=%i SEG=%i D=%i TPDU_CODE=%i PAR_FIELD=%i DEST_REF=%i\n", ext, seg, d, tpdu_code, par_field, dest_ref);
    } else if (tpdu_code == 25) {
        printf("\tI FDR EXT=%i SEG=%i D=%i TPDU_CODE=%i PAR_FIELD=%i DEST_REF=%i\n", ext, seg, d, tpdu_code, par_field, dest_ref);
    } else if (tpdu_code == 26) {
        printf("\tI DC EXT=%i SEG=%i D=%i TPDU_CODE=%i PAR_FIELD=%i DEST_REF=%i\n", ext, seg, d, tpdu_code, par_field, dest_ref);
    } else if (tpdu_code == 27) {
        printf("\tI DT EXT=%i SEG=%i D=%i TPDU_CODE=%i PAR_FIELD=%i DEST_REF=%i\n", ext, seg, d, tpdu_code, par_field, dest_ref);
    } else if (tpdu_code == 28) {
        printf("\tI DTE EXT=%i SEG=%i D=%i TPDU_CODE=%i PAR_FIELD=%i DEST_REF=%i\n", ext, seg, d, tpdu_code, par_field, dest_ref);
    } else {
        printf("\tI xxx EXT=%i SEG=%i D=%i TPDU_CODE=%i PAR_FIELD=%i DEST_REF=%i\n", ext, seg, d, tpdu_code, par_field, dest_ref);
    }

    if ((d==1) && (seg==0)) {
        data_length=bits_to_int(t+16, 8);
        tsdu_process(t+24, data_length, mod);
    }

    //TODO: segmentation

}

static void hdlc_process(const uint8_t *t, int length, int mod) {

    int hdlc, r, s, m;

    hdlc = bits_to_int(t, 8);

    if ((hdlc & 0x01) == 0) {
        r = (hdlc & 0xe0) >> 5;
        s = (hdlc & 0x0e) >> 1;
        //pe = (hdlc & 0x10) >> 4;
        printf("\tHDLC I(%i,%i)\n", r, s);
        tpdu_i_process(t+8, length-1, mod);
    } else if ((hdlc & 0x0f) == 13) {
        r = (hdlc & 0xe0) >> 5;
        printf("\tHDLC A(%i) ACK_DACH\n", r);
    } else if ((hdlc & 0x03) == 1) {
        r = (hdlc & 0xe0) >> 5;
        s = (hdlc & 0x0c) >> 2;
        printf("\tHDLC S(%i) ", r);
        switch(s) {
            case 0:
                printf("RR\n");
                break;
            case 1:
                printf("RNR\n");
                break;
            case 2:
                printf("REJ\n");
                break;
        }
    } else if ((hdlc & 0x03) == 3) {
        m = ((hdlc & 0xe0) >> 3) + ((hdlc & 0x0c) >> 2);
        printf("\tHDLC UI ");
        switch(m) {
            case 0:
                printf("UI\n");
                break;
            case 8:
                printf("DISC\n");
                break;
            case 12:
                printf("UA\n");
                break;
            case 16:
                printf("SNRM\n");
                break;
            case 20:
                printf("UI_CD\n");
                break;
            case 24:
                printf("UI_VCH\n");
                break;
            default: 
                printf("unknown\n");
                break;

        }
        tpdu_du_process(t+8, length-1, mod);
    } else {

        printf("\tHDLC xxx\n");
    }
}

void tpdu_process(const uint8_t* t, int length, int *frame_no) {

    printf("\tADDR=");
    decode_addr(t);

    hdlc_process(t+16,length-2, *frame_no);
}
