#include <stdio.h>


int bits_to_int(char *bits, int num) {

	int i, ret=0;

	for (i=0; i<num; i++)
		ret = ret + (bits[i] << (num - i - 1));

	return ret;
}

void decode_addr(char *t) {
	int x,y,z;

	z=bits_to_int(t, 1);
	y=bits_to_int(t+1, 3);
	x=bits_to_int(t+4, 12);

	if ((z==1) && (y==0))
		printf("RTI:%03x\n", x);
	if ((z==0) && (y==0))
		printf("CGI:%03x\n", x);
	if ((z==0) && (y!=0) && (y!=1)) {
		printf("TTI:%1x%03x",y, x);
		if ((y==7) && (x==0))
			printf(" no ST");
		if ((y==7) && (x==4095))
			printf(" all STs");
		printf("\n");
	}
	if (y==1)
		printf("COI:%03x\n", x);
}
