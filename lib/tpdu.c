#include "misc.h"
#include "tsdu.h"
#include "tpdu.h"
#include "misc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    /// time_ref_t - FIXME clean old incomplete messages after T454
    uint8_t id_tsap;
    uint8_t prio;
    uint8_t nsegments;  ///< total amount of segments in DU
    hdlc_frame_t *hdlc_frs[SYS_PAR_N452];
} segmented_du_t;

struct _tpdu_ui_t {
    frame_type_t fr_type;
    segmented_du_t *seg_du[128];
    tsdu_t *tsdu;           ///< contains last decoded TSDU
};

tpdu_ui_t *tpdu_ui_create(frame_type_t fr_type)
{
    if (fr_type != FRAME_TYPE_DATA && fr_type != FRAME_TYPE_HR_DATA) {
        printf("TSDU DU: usnupported frame type %d\n", fr_type);
        return NULL;
    }

    tpdu_ui_t *tpdu = malloc(sizeof(tpdu_ui_t));
    if (!tpdu) {
        return NULL;
    }
    memset(tpdu, 0, sizeof(tpdu_ui_t));
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
        for (int j = 0; j < SYS_PAR_N452; ++j) {
            free(tpdu->seg_du[i]->hdlc_frs[j]);
        }
        free(tpdu->seg_du[i]);
    }
    free(tpdu);
}

hdlc_frame_t *tpdu_ui_push_hdlc_frame(tpdu_ui_t *tpdu, hdlc_frame_t *hdlc_fr)
{
    if (hdlc_fr->nbits < 8) {
        printf("WTF too short HDLC (%d)\n", hdlc_fr->nbits);
        return false;
    }

    bool ext                    = get_bits(1, hdlc_fr->data, 0);
    const bool seg              = get_bits(1, hdlc_fr->data, 1);
    const uint8_t prio          = get_bits(2, hdlc_fr->data, 2);
    const uint8_t id_tsap       = get_bits(4, hdlc_fr->data, 4);

    printf("\tDU EXT=%d SEG=%d PRIO=%d ID_TSAP=%d", ext, seg, prio, id_tsap);
    if (ext == 0 && seg == 0) {
        tsdu_destroy(tpdu->tsdu);

        printf("\n");
        // PAS 0001-3-3 9.5.1.2
        if ((tpdu->fr_type == FRAME_TYPE_DATA && hdlc_fr->nbits > (3*8)) ||
                (tpdu->fr_type == FRAME_TYPE_DATA && hdlc_fr->nbits > (6*8))) {
            const int nbits     = get_bits(8, hdlc_fr->data + 1, 0) * 8;
            tpdu->tsdu = tsdu_d_decode(hdlc_fr->data + 2, nbits, prio, id_tsap);
        } else {
            const int nbits = hdlc_fr->nbits - 8;
            tpdu->tsdu = tsdu_d_decode(hdlc_fr->data + 1, nbits, prio, id_tsap);
        }
        return hdlc_fr;
    }

    if (ext != 1) {
        printf("\nTPDU UI: WTF, unsupported ext and seg combination\n");
        return hdlc_fr;
    }

    ext                         = get_bits(1, hdlc_fr->data + 1, 0);
    uint8_t seg_ref             = get_bits(7, hdlc_fr->data + 1, 1);
    if (!ext) {
        printf("\nTPDU UI: WTF unsupported short ext\n");
        return hdlc_fr;
    }

    ext                         = get_bits(1, hdlc_fr->data + 2, 0);
    const bool res              = get_bits(1, hdlc_fr->data + 2, 1);
    const uint8_t packet_num    = get_bits(6, hdlc_fr->data + 2, 2);
    if (ext) {
        printf("\nTPDU UI: WTF unsupported long ext\n");
        return hdlc_fr;
    }

    if (res) {
        printf("TPDU UI: WTF res != 0\n");
    }
    printf(" SEGM_REF=%d, PACKET_NUM=%d\n", seg_ref, packet_num);

    segmented_du_t *seg_du = tpdu->seg_du[seg_ref];
    if (seg_du == NULL) {
        seg_du = malloc(sizeof(segmented_du_t));
        if (!seg_du) {
            return hdlc_fr;
        }
        memset(seg_du, 0 , sizeof(segmented_du_t));
        tpdu->seg_du[seg_ref] = seg_du;
        seg_du->id_tsap = id_tsap;
        seg_du->prio = prio;
    }

    if (seg_du->hdlc_frs[packet_num]) {
        // segment already recieved
        return hdlc_fr;
    }
    seg_du->hdlc_frs[packet_num] = hdlc_fr;

    if (seg == 0) {
        seg_du->nsegments = packet_num + 1;
    }

    // last segment is still missing
    if (!seg_du->nsegments) {
        return NULL;
    }

    // check if we have all segments
    for (int i = 0; i < seg_du->nsegments; ++i) {
        if (!seg_du->hdlc_frs[i]) {
            return NULL;
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

    // free segments
    for (int i = 0; i < seg_du->nsegments; ++i) {
        if (seg_du->hdlc_frs[i] == hdlc_fr) {
            continue;
        }
        free(seg_du->hdlc_frs[i]);
    }

    free(seg_du);
    tpdu->seg_du[seg_ref] = NULL;

    printf("XXXXX\n");

    tpdu->tsdu = tsdu_d_decode(data, nbits, prio, id_tsap);

    return hdlc_fr;
}

tsdu_t *tpdu_ui_get_tsdu(tpdu_ui_t *tpdu)
{
    tsdu_t *tsdu = tpdu->tsdu;
    tpdu->tsdu = NULL;
    return tsdu;
}

bool tpdu_ui_has_tsdu(tpdu_ui_t *tpdu)
{
    return tpdu->tsdu != NULL;
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

