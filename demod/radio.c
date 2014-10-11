#include "radio.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG

int find_frame(char *buf, int buflen) {
	int i;
	char sync_byte[8] = { 0, 1, 1, 0, 0, 0, 1, 0 };
	char sync_byte2[8] = { 1, 0, 0, 1, 1, 1, 0, 1 };

	if (memcmp(buf+320, sync_byte, 8) && memcmp(buf+320, sync_byte2, 8)) return 0;
	if (memcmp(buf+160, sync_byte, 8) && memcmp(buf+160, sync_byte2, 8)) return 0;
	if (memcmp(buf, sync_byte, 8) == 0) return 1;
	if (memcmp(buf, sync_byte2, 8) == 0) return 2;
	return 0;
}

void print_buf(char *frame, int framelen) {
	int i;
	for(i=0; i<framelen; i++)
		printf("%x", frame[i]);
	printf("\n");
}

char *make_crc(char *input, int framelen)				// http://ghsi.de/CRC/index.php?Polynom=10010
   {
   char *Res = malloc(5);                                 // CRC Result
   char CRC[5];
   int  i;
   char DoInvert;
   
   for (i=0; i<5; ++i)  CRC[i] = 0;                    // Init before calculation
   
   for (i=0; i<framelen; ++i)
      {
      DoInvert = input[i] ^ CRC[4];         // XOR required?

      CRC[4] = CRC[3];
      CRC[3] = CRC[2];
      CRC[2] = CRC[1] ^ DoInvert;
      CRC[1] = CRC[0];
      CRC[0] = DoInvert;
      }

   for (i=0; i<5; ++i)
	Res[4-i] = CRC[i]; 

   return(Res);
}

int check_data_crc(char *d) {

	char *crc;
	char res;

	crc = make_crc(d, 69);
	res = memcmp(d+69, crc, 5);
//	printf("crc=");
//	print_buf(d+69,5);
//	printf("crcc=");
//	print_buf(crc,5);
	free(crc);
	if(res)
		return 0;
	else
		return 1;
}

char *decode_data_frame(char *c) {
	char *b1=malloc(26);
	char *b2=malloc(50);
	char *d=malloc(74);

	int j, check=1;

	for (j=0; j<=25; j++) b1[j] = 2;
	for (j=0; j<=49; j++) b2[j] = 2;
	for (j=0; j<=73; j++) d[j] = 2;
	
	// b'(25)=b'(-1)
	// b'(j-1)=C(2j)-C(2j+1)

	// j=0 
	b1[25]=c[0] ^ c[1];
	for(j=1; j<=25; j++) {
		b1[j-1]=c[2*j] ^ c[2*j+1];
	}

//	printf("b1=");
//	print_buf(b1,26);
	
	b2[0] = c[53];
	for(j=2; j<=48; j++) {
		b2[j-1]=c[2*j+52] ^ c[2*j+53];
	}

//	printf("b2=");
//	print_buf(b2,48);
	
	for(j=0; j<=25; j++)
		d[j]=b1[j];
	for(j=0; j<=47; j++)
		d[j+26]=b2[j];

	if ((c[150] != c[151]) || (c[148] ^ c[149] != c[150]) || (c[52] != c[53]))
		check=0;

	for (j=3; j < 23; j++) {
		if (c[2*j] != b1[j]^b1[j-1]^b1[j-2])
			check=0;
	}
	for (j=3; j < 45; j++) {
		if (c[2*j+52] != b2[j]^b2[j-1]^b2[j-2])
			check=0;
	}
	if (!check)
		d[0]=2;

	free(b1);
	free(b2);
	return d;
}


int K[] = {1, 77, 38, 114, 20, 96, 59, 135, 3, 79, 41, 117, 23, 99, 62, 138, 5, 81, 44, 120, 26, 102, 65, 141, 8, 84, 47, 123, 29, 105, 68, 144, 11, 87, 50, 126, 32, 108, 71, 147, 14, 90, 53, 129, 35, 111, 74, 150, 17, 93, 56, 132, 37, 112, 76, 148, 2, 88, 40, 115, 19, 97, 58, 133, 4, 75, 43, 118, 22, 100, 61, 136, 7, 85, 46, 121, 25, 103, 64, 139, 10, 82, 49, 124, 28, 106, 67, 142, 13, 91, 52, 127, 31, 109, 73, 145, 16, 94, 55, 130, 34, 113, 70, 151, 0, 80, 39, 116, 21, 95, 57, 134, 6, 78, 42, 119, 24, 98, 60, 137, 9, 83, 45, 122, 27, 101, 63, 140, 12, 86, 48, 125, 30, 104, 66, 143, 15, 89, 51, 128, 33, 107, 69, 146, 18, 92, 54, 131, 36, 110, 72, 149 };


char *deinterleave_frame(char *e, int framelen) {
	char *c=malloc(framelen);
	int j;
	for(j=0; j<framelen; j++) {
		c[j]=e[K[j]];
	}
	return c;

}

// inline int is_in_code(char j) {
//	char pre_cod[] = {7, 10, 13, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46, 49, 52, 55, 58, 61, 64, 67, 70, 73, 76, 83, 86, 89, 92, 95, 98, 101, 104, 107, 110, 113, 116, 119, 122, 125, 128, 131, 134, 137, 140, 143, 146, 149};
//	int i;
//	for(i=0; i<sizeof(pre_cod); i++) {
//		if(pre_cod[i]==j)
//			return 1;
//	}
//	return 0;
//}

int pre_cod[] = {1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
		
char *diffdec_frame(char *e, int framelen) {
	char *ex=malloc(framelen);
	int j;
	ex[0]=e[0]; // in spec is e[0]+f[7], but since f[7] is always 0 fuck you
	for(j=1; j<framelen; j++) {
		ex[j] = e[j] ^ e[j-pre_cod[j]];
//		if(is_in_code(j))
//			ex[j] = e[j] ^ e[j-2];
//		else
//			ex[j] = e[j] ^ e[j-1];
	}
	return ex;
}

char scramb_table[127];

int scramb(int k) {
	k%=127;
	if(k<7)
		return 1;
	return (scramb_table[k-1] ^ scramb_table[k-7]);
}

char *descramble_frame(char *f, int framelen, int scr) {
	char *ex=malloc(framelen);
	int k;
	if (scr == 0)
		memcpy(ex, f, framelen);
	else
		for(k=0; k<framelen; k++)
			ex[k]=f[k] ^ scramb_table[(k+scr)%127];
	return ex;
}

void radio_init() {
	int i;
	for(i=0; i<127; i++) {
		scramb_table[i]=scramb(i);
	}
}

void radio_process_frame(char *f, int framelen, int modulo) {

	int scr, i, j;
	char *ex, *e, *c, *d=0;

//	printf("s=");
//	print_buf(scramb_table,127);
//	printf("f=");
//	print_buf(f,160);

//	printf("Attempting descramble\n");
	int scr_ok=0;
	for(scr=0; scr<=127; scr++) {
//		printf("trying scrambling %i\n", scr);
		if(d)
			free(d);

		ex=descramble_frame(f+8, 152, scr);
//		printf("ex=");
//		print_buf(ex,152);

		e=diffdec_frame(ex, 152);
//		printf("e=");
//		print_buf(e,152);

		c=deinterleave_frame(e, 152);
//		printf("c=");
//		print_buf(c,152);

		d=decode_data_frame(c);
//		printf("d=");
//		print_buf(d,74);

		if(d[0]!=1) {
//			printf("not data frame!\n");
			goto cleanup;
		}

		if(!check_data_crc(d)) {
//			printf("crc mismatch!\n");
			goto cleanup;
		}
//		printf("b=");
//		print_buf(d+1, 68);
		
		char asbx, asby, fn0, fn1;
		asbx=d[67];			// maybe x=68, y=67
		asby=d[68];
		fn0=d[2];
		fn1=d[1];
		printf("OK mod=%03i fn=%i%i asb=%i%i scr=%03i ", modulo, fn0, fn1, asbx, asby, scr);
		for (i=0; i<8; i++) {
			for(j=0; j<8; j++)
				printf("%i", d[i*8+7-j+3]);
			printf(" ");
		}
		for (i=0; i<8; i++) {
			for(j=0; j<8; j++)
				printf("%i", d[i*8+7-j+3]);
		}
		printf("\n");
		
		scr_ok++;

	cleanup:
		free(c);
		free(e);
		free(ex);

	}
	if(scr_ok==0)
		printf("ERR2 mod=%03i\n", modulo);
	
	free(d);

}
