// I got this code from AT&T and changed it until I could read it, but the logic is theirs.

unsigned char crc8 (unsigned char *ptr, int len)
   {
   static int tblInitialized = 0;
   static unsigned char crcReg, crcTbl[256];
   unsigned long i, j, k;
   
   if( !tblInitialized )
      {
      for( i=0; i < 256; i++ )
         {
         j = i;
         for( k=8; k--; )
            j = j & 0x80? (j << 1) ^ 0x0107 : j << 1;

         crcTbl[ i ] = (unsigned char)j;
         }
      
      tblInitialized = 1;
      }
   
   crcReg = 0;
   
   for( ;len--; )
      crcReg = crcTbl[ crcReg ^ *ptr++ ];
   
   return crcReg;
   }
