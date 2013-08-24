#include <stdlib.h>
#include <string.h>
#include "crc.h"



char szBase64EncodeTableG[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+-";

unsigned char Base64CharToValue (char ch)
   {
   if (ch >= '0' && ch <= '9')
      return ch - '0';
   else if (ch >= 'a' && ch <= 'z')
      return ch - 'a' + 10;
   else if (ch >= 'A' && ch <= 'Z')
      return ch - 'A' + 36;
   else if ('+' == ch)
      return 62;
   else
      return 63;
   }

unsigned int Base64Len (int len)
   {
   len *= 4;
   return len / 3 + ((len % 3) > 0);
   }

void SecureBase64Encode (unsigned char *pby, int cby, char *psz, int cbszMax)
   {
   //assert(cbszMax >= (((cby + 1) * 4) / 3);

   unsigned char *pb = (unsigned char *)calloc(cby + 1 + 1, 1);   // data plus crc plus blank byte (to simplify encoding)
   memcpy(pb, pby, cby);
   pb[cby] = crc8(pby, cby);

   int c = Base64Len(cby + 1);
   int i;
   int iby = 0;
   for (i = 0; i < c; ++i)
      {
      switch (i % 4)
         {
         case 0:
            psz[i] = szBase64EncodeTableG[pb[iby] >> 2];  // First character is the leftmost 6 bits, leaving 2
            break;
         case 1:
            psz[i] = szBase64EncodeTableG[((pb[iby] & 0x03) << 4) | (pb[iby + 1] >> 4)]; // Second is remaining 2 lsb plus 4 msb of next
            iby++;
            break;
         case 2:
            psz[i] = szBase64EncodeTableG[((pb[iby] & 0x0F) << 2) | (pb[iby + 1] >> 6)]; // Third is remaing 4 lsb plus 2 msb of next
            iby++;
            break;
         case 3:
            psz[i] = szBase64EncodeTableG[pb[iby] & 0x3F];  // Fourth is remaining 6 lsb
            iby++;
            break;
         }
      }

   psz[i] = '\0';

   free(pb);

   return;
   }

bool SecureBase64Decode (unsigned char *pby, int cby, char *psz)
   {
   if (Base64Len(cby + 1) != strlen(psz))
      return false;

   unsigned char *pb = (unsigned char *)calloc(strlen(psz), 1); // This is longer than we need, but better than a boundary condition error.

   int iby = 0;
   for (unsigned int i = 0; i < strlen(psz); ++i)
      {
      unsigned char nVal = Base64CharToValue(psz[i]);

      switch (i % 4)
         {
         case 0:
            pb[iby] = nVal << 2;
            break;
         case 1:
            pb[iby] |= (nVal & 0x30) >> 4;
            iby++;
            pb[iby] = (nVal & 0x0F) << 4;
            break;
         case 2:
            pb[iby] |= (nVal & 0x3C) >> 2;
            iby++;
            pb[iby] = (nVal & 0x03) << 6;
            break;
         case 3:
            pb[iby] |= nVal;
            iby++;
            break;
         }
      }

   bool bRet = (crc8(pb, iby - 1) == pb[iby - 1]);

   memcpy(pby, pb, iby - 1);

   free(pb);

   return bRet;
   }