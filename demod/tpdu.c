#include <stdio.h>
#include <string.h>
#include "tsdu.h"


char stuff[] = {1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1};

int detect_stuff(char *bits) {
	int i;

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

void hdlc_process(char *t, int length, int mod) {

	int hdlc;

	hdlc = bits_to_int(t, 8);

	if (hdlc == 3) {
		printf("\tHDLC UI\n");
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

