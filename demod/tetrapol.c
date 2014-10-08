#include <stdio.h>
#include "constants.h"
#include "radio.h"
#include <string.h>

int main(int argc, char* argv[]) {
	int c=0;
	int oldc=0;
	int i, found;
	char buf[FRAME_LEN*3];
	int pos=1-FRAME_LEN*3;
	int frameno=0;
	bzero(buf, FRAME_LEN*3);
	radio_init();
	if (argc > 1)
		fread(buf, atoi(argv[1]), 1, stdin);
	while(c==0 || c==1) {
		c=fgetc(stdin);
		memmove(buf, buf+1, FRAME_LEN*3-1);
//		buf[FRAME_LEN*2-1]=c;
		oldc=oldc^c;
//		printf("%i"
		buf[FRAME_LEN*3-1]=oldc;
		found = find_frame(buf, FRAME_LEN*3);
		if (pos%160 == 0)
		if(found) {
			if (found == 2)
				for (i=0; i<160; i++)
					buf[i] = buf[i] ^ 1;
			print_buf(buf, FRAME_LEN);
			frameno=pos/160;
			printf("GOT SYNC (frame no: %i, pos: %i mod: %i)!\n", frameno, pos, frameno%200);
			radio_process_frame(buf, FRAME_LEN, frameno%200);
		}
		pos++;

	}
	return 0;
}
