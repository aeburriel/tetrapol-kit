#include <stdio.h>
#include <string.h>
#include "misc.h"
#include "tsdu.h"
#include "radio.h"
#include "misc.h"


char stuff[] = {1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1};

int detect_stuff(char *bits) {
    int i;

    //	print_buf(bits,40);
    //	print_buf(stuff,80);
    for (i=0; i<40; i++)
        if (memcmp(bits, stuff+i, 40) == 0)
            return 1;
    return 0;

}



char segbuf[10000];
int numoctets, startmod;

void segmentation_reset() {
    numoctets=0;
}

void tpdu_du_process(char* t, int length, int mod) {

    int ext, seg, prio, id_tsap;
    int data_length=0;
    int segmentation_reference, packet_number;

    if (detect_stuff(t)) {
        printf("\tSTUFFED\n");
        return;
    }

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

void tpdu_i_process(char* t, int length, int mod) {

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

void hdlc_process(char *t, int length, int mod) {

    int hdlc, r, s, pe, m;

    hdlc = bits_to_int(t, 8);

    if ((hdlc & 0x01) == 0) {
        r = (hdlc & 0xe0) >> 5;
        s = (hdlc & 0x0e) >> 1;
        pe = (hdlc & 0x10) >> 4;
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

void tpdu_process(char* t, int length, int mod) {

    printf("\tADDR=");
    decode_addr(t);


    if (((mod==-1) && (length==24)) || (mod==0) || (mod==100)) {
        decode_bch(t);
        return;
    }

    //	if (mod==-1)
    //		return;

    if ((mod==98) ||(mod==198)) {
        decode_pch(t);
        return;
    }

    if (mod%25 == 14) {
        decode_rch(t);
        return;
    }

    hdlc_process(t+16,length-2, mod);

}

