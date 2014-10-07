// ==========================================================================
// CRC Generation Unit - Linear Feedback Shift Register implementation
// (c) Kay Gorontzi, GHSi.de, distributed under the terms of LGPL
// ==========================================================================
char *MakeCRC(char *BitString)
   {
   static char Res[6];                                 // CRC Result
   char CRC[5];
   int  i;
   char DoInvert;
   
   for (i=0; i<5; ++i)  CRC[i] = 0;                    // Init before calculation
   
   for (i=0; i<strlen(BitString); ++i)
      {
      DoInvert = ('1'==BitString[i]) ^ CRC[4];         // XOR required?

      printf("%i %i %i\t", i, ('1'==BitString[i]), DoInvert);


      CRC[4] = CRC[3];
      CRC[3] = CRC[2];
      CRC[2] = CRC[1] ^ DoInvert;
      CRC[1] = CRC[0];
      CRC[0] = DoInvert;

      printf("%i %i %i %i %i\n", CRC[0], CRC[1], CRC[2], CRC[3], CRC[4]);
      }
      
   for (i=0; i<5; ++i)  Res[4-i] = CRC[i] ? '1' : '0'; // Convert binary to ASCII
   Res[5] = 0;                                         // Set string terminator

   return(Res);
   }

// A simple test driver:

#include <stdio.h>

int main(int argc, char* argv[])
   {
   char *Data, *Result;                                       // Declare two strings

   Data = argv[1];
   Result = MakeCRC(Data);                                    // Calculate CRC
   
   printf("CRC of [%s] is [%s] with P=[100101]\n", Data, Result);
   
   return(0);
   }

