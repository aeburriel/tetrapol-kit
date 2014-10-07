#include <stdio.h>


int main() {

char x[31];
int i;

x[0]=0;
x[1]=0;
x[2]=0;
x[3]=0;
x[4]=1;

for (i=5; i<31; i++) 
	x[i] = x[i-3] ^ x[i-5];

for (i=0; i<31; i++) 
	printf("%i", x[i]);
printf("\n");
}

	
