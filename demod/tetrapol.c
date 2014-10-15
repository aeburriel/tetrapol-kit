#include <stdio.h>
#include <string.h>
#include "constants.h"
#include "radio.h"
#include "multiblock.h"

int mod;

int main(int argc, char* argv[]) {
	int c=0;
	int oldc=0;
	int i, found=0, sync_lost=1;
	char buf[FRAME_LEN*3];
	int pos=1-FRAME_LEN*3;
	int frameno=0;
	int sync_keep=50;

	bzero(buf, FRAME_LEN*3);
	radio_init();
	mod=-1;

	while(c==0 || c==1) {
		c=fgetc(stdin);
		memmove(buf, buf+1, FRAME_LEN*3-1);
		oldc=oldc^c;
		buf[FRAME_LEN*3-1]=oldc;
		if (pos%160 == 0 || sync_lost >= sync_keep) {
			if (mod != -1)
				mod++;
			if (mod==200)
				mod=0;
			found = find_frame(buf, FRAME_LEN*3);
			if ((sync_lost > sync_keep) && found) {
				printf("Got sync\n");
				pos=0;
			}

			if (sync_lost == sync_keep)
				printf("Sync lost\n");
			if (!found && (pos%160 == 0) && sync_lost < sync_keep) {
				printf("ERR1 mod=%03i, lost=%i\n", mod, sync_lost);
				multiblock_reset();
			}
		
			if (found)
				sync_lost=0;
			else
				sync_lost++;
			if(found) {
				if (found == 2)
					for (i=0; i<160; i++)
						buf[i] = buf[i] ^ 1;
//				print_buf(buf, FRAME_LEN);
				frameno=pos/160;
//				printf("GOT SYNC (frame no: %i, pos: %i mod: %i)!\n", frameno, pos, mod);
				radio_process_frame(buf, FRAME_LEN, mod);
			}
		}
		pos++;

	}
	return 0;
}

void mod_set(int m) {
	
	mod=m;
}
