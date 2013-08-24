#include "mbcmif.h"
#include <stdlib.h>

#include <windows.h> // TODO: Remove this 



// This array maps the neighbors of a tile to what water tile should be used.
// This is based on a system with the most significant bit in the upper left
// and the least significant in the lower right. A zero bit means a water
// neighbor and a one bit means a land neighbor. Some examples:
//
// WWW
// WWW = index 0x00 in the array, a full water tile
// WWW
//
// .W.
// WWW = index 0xA5 in the array, an interesction tile
// .W.
//
// ...
// .W. = index 0xFF in the array, a puddle tile
// ...
//
// To say that this was tedious to generate doesn't really tell the whole story...

BYTE aWaterTileMapG[] = {
      0x44, 0x40, 0x32, 0x32, 0x3C, 0x38, 0x32, 0x32, // 0x00
      0x2C, 0x2C, 0x26, 0x26, 0x2A, 0x2A, 0x26, 0x26,
      0x2F, 0x2D, 0x27, 0x27, 0x2F, 0x2D, 0x27, 0x27, // 0x10
      0x1B, 0x1B, 0x49, 0x49, 0x1B, 0x1B, 0x49, 0x49,
      0x42, 0x3E, 0x30, 0x30, 0x3A, 0x36, 0x30, 0x30, // 0x20
      0x2C, 0x2C, 0x26, 0x26, 0x2A, 0x2A, 0x26, 0x26,
      0x2E, 0x22, 0x1E, 0x1E, 0x2E, 0x22, 0x1E, 0x1E, // 0x30
      0x1B, 0x1B, 0x49, 0x49, 0x1B, 0x1B, 0x49, 0x49,
      0x35, 0x33, 0x1C, 0x1C, 0x34, 0x24, 0x1C, 0x1C, // 0x40
      0x28, 0x28, 0x4B, 0x4B, 0x1F, 0x1F, 0x4B, 0x4B,
      0x29, 0x20, 0x4C, 0x4C, 0x29, 0x20, 0x4C, 0x4C, // 0x50
      0x48, 0x48, 0x4A, 0x4A, 0x48, 0x48, 0x4A, 0x4A,
      0x35, 0x33, 0x1C, 0x1C, 0x34, 0x24, 0x1C, 0x1C, // 0x60
      0x28, 0x28, 0x4B, 0x4B, 0x1F, 0x1F, 0x4B, 0x4B,
      0x29, 0x20, 0x4C, 0x4C, 0x29, 0x20, 0x4C, 0x4C, // 0x70
      0x48, 0x48, 0x4A, 0x4A, 0x48, 0x48, 0x4A, 0x4A,
      0x43, 0x3F, 0x31, 0x31, 0x3B, 0x37, 0x31, 0x31, // 0x80
      0x2B, 0x2B, 0x1D, 0x1D, 0x21, 0x21, 0x1D, 0x1D,
      0x2F, 0x2D, 0x27, 0x27, 0x2F, 0x2D, 0x27, 0x27, // 0x90
      0x1B, 0x1B, 0x49, 0x49, 0x1B, 0x1B, 0x49, 0x49,
      0x41, 0x3D, 0x23, 0x23, 0x39, 0x25, 0x23, 0x23, // 0xA0
      0x2B, 0x2B, 0x1D, 0x1D, 0x21, 0x21, 0x1D, 0x1D,
      0x2E, 0x22, 0x1E, 0x1E, 0x2E, 0x22, 0x1E, 0x1E, // 0xB0
      0x1B, 0x1B, 0x49, 0x49, 0x1B, 0x1B, 0x49, 0x49,
      0x35, 0x33, 0x1C, 0x1C, 0x34, 0x24, 0x1C, 0x1C, // 0xC0
      0x28, 0x28, 0x4B, 0x4B, 0x1F, 0x1F, 0x4B, 0x4B,
      0x29, 0x20, 0x4C, 0x4C, 0x29, 0x20, 0x4C, 0x4C, // 0xD0
      0x48, 0x48, 0x4A, 0x4A, 0x48, 0x48, 0x4A, 0x4A,
      0x35, 0x33, 0x1C, 0x1C, 0x34, 0x24, 0x1C, 0x1C, // 0xE0
      0x28, 0x28, 0x4B, 0x4B, 0x1F, 0x1F, 0x4B, 0x4B,
      0x29, 0x20, 0x4C, 0x4C, 0x29, 0x20, 0x4C, 0x4C, // 0xF0
      0x48, 0x48, 0x4A, 0x4A, 0x48, 0x48, 0x4A, 0x4A};
   
int magic(int x, int y)
   {
   static BYTE matrix[8][8] = {
      { 2, 0, 2, 1, 3, 0, 3, 1 } ,
      { 3, 1, 0, 3, 2, 1, 0, 2 } , 
      { 0, 2, 1, 2, 0, 3, 1, 3 } , 
      { 1, 3, 0, 3, 1, 2, 0, 2 } , 
      { 2, 0, 1, 2, 3, 0, 1, 3 } , 
      { 3, 1, 3, 0, 2, 1, 2, 0 } , 
      { 0, 2, 0, 1, 3, 0, 3, 2 } , 
      { 1, 3, 0, 3, 2, 1, 2, 0 }	};
      
      return matrix[y % 8][x % 8];
   }
   
   
   
MBCMIF::MBCMIF ()
   {
   /*int x, y;
   
   for (y = 0; y < MAX_TILE_COUNT; ++y)
      for (x = 0; x < MAX_TILE_COUNT; ++x)
         {
         aaCornerMap[x][y] = 0;
         aaCenterMap[x][y] = '@';
         }*/
   }

void MBCMIF::GenerateMap(MBCMapImage& mi)
   {
   mi.cTilesX = nDimension;
   mi.cTilesY = nDimension;
   mi.nTileset = nTileset;
   strcpy(mi.szName, szName);
   
   int x, y;
   for (y = 0; y < nDimension ; ++y)
      for (x = 0; x < nDimension; ++x)
         mi.aaTiles[x][y] = FindTileFor(x, y);
      
   DefineEnergyPools(mi);
   DefineStartLocations(mi);
   MakeCraters(mi);
   }
/*
void MBCMIF::BuildCalibrationMap (void)
   {
   nVersion = 3;
   nDimension = 32;
   nTileset = 1;
   strcpy(szName, "Calibration");
   
   int x, y;
   for (x = 0; x < 32; ++x)
      for (y = 0; y < 32; ++y)
         {
         aaCornerMap[x][y] = 0;
         aaCenterMap[x][y] = '.';
         }
      
   aaCenterMap[0][0] = 'S';
   aaCenterMap[2][0] = 'M';
   aaCenterMap[4][0] = 'L';
   
   aaCenterMap[7][7] = -1;
   aaCenterMap[23][7] = -1;
   aaCenterMap[7][23] = -1;
   aaCenterMap[23][23] = -1;
   }
*/   
#ifndef MOONBASECONSOLE
void MBCMIF::Read_MIF_File (FILE *pf)
   {
   int rc;
   char szSize[7];
   
   // First line should always be the header
   rc = fscanf(pf, "MBCMIF%d", &nVersion);
   if (rc != 1)
      {
      fprintf(stderr, "ERROR: Bad file header. Header must start with MBCMIF. Now exiting.\n");
      exit(-1);
      }
   
   // Second line should always be the size
   if (nVersion == 1 || nVersion == 2)
      {
      rc = fscanf(pf, "%6s", &szSize);
      // TODO: Check return code.
      
      if (!strcmp("SMALL", szSize))
         nDimension = 32;
      else if (!strcmp("MEDIUM", szSize))
         nDimension = 40;
      else if (!strcmp("LARGE", szSize))
         nDimension = 48;
      else if (!strcmp("HUGE", szSize))
         nDimension = 56;
      else if (!strcmp("SAI", szSize))
         nDimension = 64;
      else if (!strcmp("MAX", szSize))
         nDimension = 80;
      else
         {
         fprintf(stderr, "The size \"%s\" is unsupported.\n", szSize);
         exit(-1);
         }
      }
   else if (nVersion == 3)
      fscanf(pf, "%d", &nDimension);
   
   // Third and fourth lines aren't present in version one, they are the tileset and name in later versions.
   if (nVersion == 1)
      {
      nTileset = 1;
      strcpy(szName, "AutoGenerated");
      }
   if (nVersion >= 2)
      {
      char ch;
      
      // Read the tileset number and skip to the end of the line.
      rc = fscanf(pf, "%d", &nTileset);
      fread(&ch, sizeof(char), 1, pf);
      while (ch != 0x0A && !feof(pf))
         fread(&ch, sizeof(char), 1, pf);
      
      // Read the name, limiting it to 16 characters and consuming the rest of the line
      int i = 0;
      
      memset(szName, 0, 17);
      fread(&ch, sizeof(char), 1, pf);
      while (ch != 0x0A && ch != 0xD && i < 16)
         {
         szName[i++] = ch;
         fread(&ch, sizeof(char), 1, pf);
         }
      while (ch != 0x0A && !feof(pf))
         fread(&ch, sizeof(char), 1, pf);
      szName[16] = '\0';
      }
   
   // Read the actual map data
   int y;
   int x;
   char ch;
   
   // TODO: Make this more robust
   for (y = 0; y < nDimension; ++y)
      {
      int corner;
      
      for (x = 0; x < nDimension; ++x)
         {
         // Need to do it as two lines because we can't just scan one byte as a number with fscanf.
         fscanf(pf, "%d", &corner);
         aaCornerMap[x][y] = corner;
         }
      
      // Skip the linefeed left by the last line <shudder>.
      fread(&ch, sizeof(char), 1, pf);
      while (0x0A != ch && !feof(pf))
         fread(&ch, sizeof(char), 1, pf);
      for (x = 0; x < nDimension; ++x)
         {
         int nStartLocMask = -1;
         
         fread(&ch, sizeof(char), 1, pf);
         if (ch != ' ')
            if (ch >= '0' && ch <= '9')
               nStartLocMask = (ch - '0') * 16;
            else if (ch >= 'A' && ch <= 'F')
               nStartLocMask = (10 + ch - 'A') * 16;
            
            fread(&ch, sizeof(char), 1, pf);
            
            if (nStartLocMask > -1)
               {
               if (ch >= '0' && ch <= '9')
                  nStartLocMask += ch - '0';
               else if (ch >= 'A' && ch <= 'F')
                  nStartLocMask += 10 + ch - 'A';
               
               aaCenterMap[x][y] = -nStartLocMask;
               }
            else
               aaCenterMap[x][y] = ch;
         }
      }
   }
#endif
   
void MBCMIF::DefineStartLocations (MBCMapImage& mi)
   {
   int x, y;
   
   for (y = 0; y < nDimension ; ++y)
      for (x = 0; x < nDimension; ++x)
         {
         char ch = aaCenterMap[x][y];
         
         if (ch < 0)
            {
            int i; 
            ch = -ch;
            
            if (ch & 1)
               {
               // 4 player start
               i = 0;
               while (i < 4)
                  {
                  if (mi.apl4PLocs[i].xLoc == 0xFFFF)
                     {
                     mi.apl4PLocs[i].xLoc = x * 60;
                     mi.apl4PLocs[i].yLoc = y * 60;
                     break;
                     }
                  ++i;
                  }
               }
            ch = ch >> 1;
            if (ch & 1)
               {
               // 3 player start
               i = 0;
               while (i < 3)
                  {
                  if (mi.apl3PLocs[i].xLoc == 0xFFFF)
                     {
                     mi.apl3PLocs[i].xLoc = x * 60;
                     mi.apl3PLocs[i].yLoc = y * 60;
                     break;
                     }
                  ++i;
                  }
               }
            ch = ch >> 1;
            if (ch & 1)
               {
               // 2 player start
               i = 0;
               while (i < 2)
                  {
                  if (mi.apl2PLocs[i].xLoc == 0xFFFF)
                     {
                     mi.apl2PLocs[i].xLoc = x * 60;
                     mi.apl2PLocs[i].yLoc = y * 60;
                     break;
                     }
                  ++i;
                  }
               }
            ch = ch >> 1;
            if (ch & 1)
               {
               // 2v2 player start
               i = 0;
               while (i < 4)
                  {
                  if (mi.apl2v2PLocs[i].xLoc == 0xFFFF)
                     {
                     mi.apl2v2PLocs[i].xLoc = x * 60;
                     mi.apl2v2PLocs[i].yLoc = y * 60;
                     break;
                     }
                  ++i;
                  }
               }
            ch = ch >> 1;
            if (ch & 1)
               {
               // 1v3 player start
               i = 0;
               while (i < 4)
                  {
                  if (mi.apl1v3PLocs[i].xLoc == 0xFFFF)
                     {
                     mi.apl1v3PLocs[i].xLoc = x * 60;
                     mi.apl1v3PLocs[i].yLoc = y * 60;
                     break;
                     }
                  ++i;
                  }
               }
            ch = ch >> 1;
            if (ch & 1)
               {
               // 1v2 player start
               i = 0;
               while (i < 3)
                  {
                  if (mi.apl1v2PLocs[i].xLoc == 0xFFFF)
                     {
                     mi.apl1v2PLocs[i].xLoc = x * 60;
                     mi.apl1v2PLocs[i].yLoc = y * 60;
                     break;
                     }
                  ++i;
                  }
               }
            }
         }
   }
   
void MBCMIF::DefineEnergyPools (MBCMapImage& mi)
   {
   int x, y;
   
   for (y = 0; y < nDimension ; ++y)
      for (x = 0; x < nDimension; ++x)
         {
         char ch = aaCenterMap[x][y];
         
         if ('S' == ch || 'M' == ch || 'L' == ch)
            {
            // Verify legal position
            if (!((TLCorner(x,y) == TRCorner(x,y)) && (BLCorner(x,y) == BRCorner(x,y)) &&
               (TLCorner(x,y) == BLCorner(x,y)) && (TRCorner(x,y) == BRCorner(x,y))))
               throw CMapError(x, y, "small and medium energy pools must be on a flat tile");
            
            if ('L' == ch)
               {
               BYTE nHeight;
               nHeight = BLCorner(x,y);
               if (!(TLCorner(x,y) == nHeight && TTLCorner(x,y) == nHeight && TTRCorner(x,y) == nHeight && TRCorner(x,y) == nHeight && BRCorner(x,y) == nHeight))
                  throw CMapError(x, y, "large energy pools must be on the lower of two flat tiles");
               }
            
            int xLoc;
            int yLoc;
            
            if ('S' == ch)
               {
               xLoc = 60 * x + 30 + 20000;
               yLoc = 60 * y + 30;
               }
            else if ('M' == ch)
               {
               xLoc = 60 * x + 30 + 10000;
               yLoc = 60 * y + 30;
               }
            else
               {
               xLoc = 60 * x + 30;
               yLoc = 60 * y;
               }
            
            if (mi.cEnergyPools < 49)
               {
               mi.aepLocs[mi.cEnergyPools].plLocation.xLoc = xLoc;
               mi.aepLocs[mi.cEnergyPools].plLocation.yLoc = yLoc;
               
               ++mi.cEnergyPools;
               }
            else if (mi.cEnergyPools == 49)
               {
               mi.epLastPool.xLoc = xLoc;
               mi.epLastPool.yLoc = yLoc;
               
               ++mi.cEnergyPools;
               }
            else
               throw CMapError(x, y, "only 50 energy pools are allowed, this is the 51st");
            }
         }
   }
   
void MBCMIF::MakeCraters (MBCMapImage& mi)
   {
   //squarenumber, type, x, y (offset from top left (abs y)). x/y = 9 if none of that type
   BYTE locations[8][3][2] = {{ {1,1}, {5,2}, {3,5} },
                              { {6,1}, {1,6}, {2,0} },
                              { {0,4}, {3,2}, {6,5} },
                              { {4,4}, {5,0}, {9,9} },
                              { {3,6}, {9,9}, {2,1} },
                              { {9,9}, {3,3}, {0,2} },
                              { {2,4}, {0,0}, {5,3} },
                              { {4,1}, {0,3}, {5,6} } };
   
   BYTE largegrid[10][10] =  {{0,1,2,3,4,5,6,7,0,1}, // I made up the crater patterns for sizes larger than SAI
                              {2,3,4,5,6,7,0,1,2,3}, // This will work for maps up to 80x80
                              {4,5,6,7,0,1,2,3,4,5},
                              {3,0,1,2,6,4,5,7,3,0},
                              {1,2,3,4,5,6,7,0,1,2},
                              {3,4,5,6,7,0,1,2,3,4},
                              {6,3,0,1,2,7,4,5,6,3},
                              {5,6,7,0,1,2,3,4,5,6},
                              {0,1,2,3,4,5,6,7,0,1},
                              {2,3,4,5,6,7,0,1,2,3} };
   
   for (int i = 0; i < nDimension / 8; i++)
      for (int j = 0; j < nDimension / 8; j++)
         for (int nCrater = 0; nCrater < 3; nCrater++)
            {
            if (9 == locations[largegrid[j][i]][nCrater][0])
               continue;

            int x = locations[largegrid[j][i]][nCrater][0] + i * 8;
            int y = locations[largegrid[j][i]][nCrater][1] + j * 8;

            BYTE nLevel = TLCorner(x,y);
            if ( (TLCorner(x,y) == nLevel) &&  (TRCorner(x,y) == nLevel) &&  (TRRCorner(x,y) == nLevel) &&
                            (aaCenterMap[x][y]   != 'W') && (aaCenterMap[x+1][y]   != 'W') &&
                 (BLCorner(x,y) == nLevel) &&  (BRCorner(x,y) == nLevel) &&  (BRRCorner(x,y) == nLevel) &&
                            (aaCenterMap[x][y+1] != 'W') && (aaCenterMap[x+1][y+1] != 'W') &&
                (BBLCorner(x,y) == nLevel) && (BBRCorner(x,y) == nLevel) && (BBRRCorner(x,y) == nLevel))
               {
               // The tile values follow a predictable pattern, level one craters in order, etc.
               WORD nBase = 0xA6 + (TLCorner(x,y) * 12) + (nCrater * 4);

               mi.aaTiles[x]  [y]   = nBase;
               mi.aaTiles[x+1][y]   = nBase + 1;
               mi.aaTiles[x]  [y+1] = nBase + 2;
               mi.aaTiles[x+1][y+1] = nBase + 3;
               }
            }
   }


WORD MBCMIF::FindTileFor (int x, int y)
   {
   int index;
   char ch;
   
   BYTE aLowBlanks[] = { 0x93, 0x94, 0x00, 0x96 };
   BYTE aMedBlanks[] = { 0x97, 0x99, 0x0D, 0x9A };
   BYTE aHiBlanks[]  = { 0x9B, 0x9C, 0x1A, 0x9D };
   ch = aaCenterMap[x][y];
   if ('S' == ch || 'M' == ch || 'L' == ch || '.' == ch || ch < 0)
      {
      // Do the easy cases, things with no transitions.
      if (0 == TLCorner(x,y) && 0 == TRCorner(x,y) && 0 == BLCorner(x,y) && 0 == BRCorner(x,y))
         return aLowBlanks[magic(x,y)];
      //         return 0x00; // Default blank low level
      //         return aLevelOneBlankMap[y][x];  // TODO: Deal with partial special graphics and maps bigger than the template
      if (1 == TLCorner(x,y) && 1 == TRCorner(x,y) && 1 == BLCorner(x,y) && 1 == BRCorner(x,y))
         return aMedBlanks[magic(x,y)];
      //         return 0x0D; // Default blank medium level
      if (2 == TLCorner(x,y) && 2 == TRCorner(x,y) && 2 == BLCorner(x,y) && 2 == BRCorner(x,y))
         return aHiBlanks[magic(x,y)];
      //         return 0x1A;  // Default blank high level
      
      // 
      // Low to med transitions
      //
      if (0 == TLCorner(x,y) || 0 == TRCorner(x,y) || 0 == BLCorner(x,y) || 0 == BRCorner(x,y))
         {
         // Corner cases
         
         int CornerSum = TLCorner(x,y) + TRCorner(x,y) + BLCorner(x,y) + BRCorner(x,y);
         
         if (1 == CornerSum)
            {
            if (TLCorner(x,y))
               {
               if (TLLCorner(x,y) > 0 && TTLCorner(x,y) > 0)
                  return 0x03;
               else
                  return 0x89;
               }
            else if (TRCorner(x,y))
               {
               if (TRRCorner(x,y) > 0 && TTRCorner(x,y) > 0)
                  return 0x04;
               else
                  return 0x8C;
               }
            else if (BLCorner(x,y))
               {
               if (BLLCorner(x,y) > 0 && BBLCorner(x,y) > 0)
                  return 0x02;
               else
                  return 0x86;
               }
            else // BRCorner
               {
               if (BRRCorner(x,y) > 0 && BBRCorner(x,y) > 0)
                  return 0x01;
               else
                  return 0x83;
               }
            }
         
         // Straight edges
         
         // edge on bottom
         if (TLCorner(x,y) == 0 && TRCorner(x,y) == 0 && BLCorner(x,y) == 1 && BRCorner(x,y) == 1)
            {
            bool bLeftEased  = (BLLCorner(x,y) == 0 && BBLCorner(x,y) == 1);
            bool bRightEased = (BRRCorner(x,y) == 0 && BBRCorner(x,y) == 1);
            
            if ( bLeftEased &&  bRightEased)
               return 0x0A;
            if (!bLeftEased &&  bRightEased)
               return 0x54;
            if ( bLeftEased && !bRightEased)
               return 0x55;
            if (!bLeftEased && !bRightEased)
               return (magic(x,y) & 0x01) ? 0x9F : 0x56;
            }
         // edge on top
         else if (TLCorner(x,y) == 1 && TRCorner(x,y) == 1 && BLCorner(x,y) == 0 && BRCorner(x,y) == 0)
            {
            bool bLeftEased  = (TLLCorner(x,y) == 0 && TTLCorner(x,y) == 1);
            bool bRightEased = (TRRCorner(x,y) == 0 && TTRCorner(x,y) == 1);
            
            if ( bLeftEased &&  bRightEased)
               return 0x0C;
            if (!bLeftEased &&  bRightEased)
               return 0x52;
            if ( bLeftEased && !bRightEased)
               return 0x51;
            if (!bLeftEased && !bRightEased)
               return (magic(x,y) & 0x01) ? 0xA1 : 0x53;
            }
         // edge on right
         if (TLCorner(x,y) == 0 && BLCorner(x,y) == 0 && TRCorner(x,y) == 1 && BRCorner(x,y) == 1)
            {
            bool bTopEased = (TTRCorner(x,y) == 0 && TRRCorner(x,y) == 1);
            bool bBotEased = (BBRCorner(x,y) == 0 && BRRCorner(x,y) == 1);
            
            if ( bTopEased &&  bBotEased)
               return 0x09;
            if (!bTopEased &&  bBotEased)
               return 0x5B;
            if ( bTopEased && !bBotEased)
               return 0x5A;
            if (!bTopEased && !bBotEased)
               return (magic(x,y) & 0x01) ? 0x9E : 0x5C;
            }
         // edge on left
         if (TLCorner(x,y) == 1 && BLCorner(x,y) == 1 && TRCorner(x,y) == 0 && BRCorner(x,y) == 0)
            {
            bool bTopEased = (TTLCorner(x,y) == 0 && TLLCorner(x,y) == 1);
            bool bBotEased = (BBLCorner(x,y) == 0 && BLLCorner(x,y) == 1);
            
            if ( bTopEased &&  bBotEased)
               return 0x0B;
            if (!bTopEased &&  bBotEased)
               return 0x57;
            if ( bTopEased && !bBotEased)
               return 0x58;
            if (!bTopEased && !bBotEased)
               return (magic(x,y) & 0x01) ? 0xA0 : 0x59;
            }
         
         // Three corner cases
         
         // 0 1         1 1           0 0 0       0 0 0
         // 0 1 1 0x5F  0 1 1 0x5E    0 1 1 0x62  0 1 1 0x60
         // 0 0 0       0 0 0         0 1         1 1  
         //
         // 0 1         1 1           0 0 1       0 0 1
         // 0 1 1 0x5D  0 1 1 0x06    0 1 1 0x61  0 1 1 0x07
         // 0 0 1       0 0 1         0 1         1 1  
         //
         //
         // 0 0 0       1 0 0           1 0         1 1
         // 1 1 0 0x65  1 1 0 0x63    1 1 0 0x68  1 1 0 0x66
         //   1 0         1 0         0 0 0       0 0 0
         //
         // 0 0 0       1 0 0           1 0         1 1
         // 1 1 0 0x64  1 1 0 0x08    1 1 0 0x67  1 1 0 0x05
         //   1 1         1 1         1 0 0       1 0 0
         
         // corner in upper left
         if (BLCorner(x,y) == 1 && TLCorner(x,y) == 1 && TRCorner(x,y) == 1)
            {
            bool BLDiag = (BLLCorner(x,y) > 0 && BBLCorner(x,y) == 0);
            bool TRDiag = (TTRCorner(x,y) > 0 && TRRCorner(x,y) == 0);
            
            if      (!BLDiag && !TRDiag)
               return 0x62;
            else if (!BLDiag &&  TRDiag)
               return 0x61;
            else if ( BLDiag && !TRDiag)
               return 0x60;
            else
               return 0x07;
            }
         
         // corner in upper right
         if (TLCorner(x,y) == 1 && TRCorner(x,y) == 1 && BRCorner(x,y) == 1)
            {
            bool TLDiag = (TTLCorner(x,y) > 0 && TLLCorner(x,y) == 0);
            bool BRDiag = (BRRCorner(x,y) > 0 && BBRCorner(x,y) == 0);
            
            if      (!TLDiag && !BRDiag)
               return 0x65;
            else if (!TLDiag &&  BRDiag)
               return 0x64;
            else if ( TLDiag && !BRDiag)
               return 0x63;
            else
               return 0x08;
            }
         
         // corner in bottom right
         if (TRCorner(x,y) == 1 && BRCorner(x,y) == 1 && BLCorner(x,y) == 1)
            {
            bool TRDiag = (TRRCorner(x,y) > 0 && TTRCorner(x,y) == 0);
            bool BLDiag = (BBLCorner(x,y) > 0 && BLLCorner(x,y) == 0);
            
            if      (!TRDiag && !BLDiag)
               return 0x68;
            else if (!TRDiag &&  BLDiag)
               return 0x67;
            else if ( TRDiag && !BLDiag)
               return 0x66;
            else
               return 0x05;
            }
         
         // corner in bottom left
         if (BRCorner(x,y) == 1 && BLCorner(x,y) == 1 && TLCorner(x,y) == 1)
            {
            bool TLDiag = (TLLCorner(x,y) > 0 && TTLCorner(x,y) == 0);
            bool BRDiag = (BBRCorner(x,y) > 0 && BRRCorner(x,y) == 0);
            
            if      (!TLDiag && !BRDiag)
               return 0x5F;
            else if (!TLDiag &&  BRDiag)
               return 0x5D;
            else if ( TLDiag && !BRDiag)
               return 0x5E;
            else
               return 0x06;
            }
         
         // Opposing corner cases
         if (TLCorner(x,y) == 1 && BRCorner(x,y) == 1)
            {
            // There are four cases, big big, big small, small big, small small
            
            // big big
            if (TLLCorner(x,y) > 0 && TTLCorner(x,y) > 0 && BRRCorner(x,y) > 0 && BBRCorner(x,y) > 0)
               return 0x4D;
            // big small
            if (TLLCorner(x,y) > 0 && TTLCorner(x,y) > 0)
               return 0x81;
            // small big
            if (BRRCorner(x,y) > 0 && BBRCorner(x,y) > 0)
               return 0x82;
            // small small
            return 0x84;
            }
         if (TRCorner(x,y) == 1 && BLCorner(x,y) == 1)
            {
            // There are four cases, big big, big small, small big, small small
            
            // big big
            if (TRRCorner(x,y) > 0 && TTRCorner(x,y) > 0 && BLLCorner(x,y) > 0 && BBLCorner(x,y) > 0)
               return 0x4E;
            // big small
            if (TRRCorner(x,y) > 0 && TTRCorner(x,y) > 0)
               return 0x85;
            // small big
            if (BLLCorner(x,y) > 0 && BBLCorner(x,y) > 0)
               return 0x87;
            // small small
            return 0x88;
            }
         }
         
         // 
         // Med to high transitions
         //
         if (1 == TLCorner(x,y) || 1 == TRCorner(x,y) || 1 == BLCorner(x,y) || 1 == BRCorner(x,y))
            {
            // Corner cases
            
            int CornerSum = (TLCorner(x,y) == 2) + (TRCorner(x,y) == 2) + (BLCorner(x,y) == 2) + (BRCorner(x,y) == 2);
            
            if (1 == CornerSum)
               {
               if (TLCorner(x,y) == 2)
                  {
                  if (TLLCorner(x,y) == 2 && TTLCorner(x,y) == 2)
                     return 0x10;
                  else
                     return 0x95;
                  }
               else if (TRCorner(x,y) == 2)
                  {
                  if (TRRCorner(x,y) == 2 && TTRCorner(x,y) == 2)
                     return 0x11;
                  else
                     return 0x98;
                  }
               else if (BLCorner(x,y) == 2)
                  {
                  if (BLLCorner(x,y) == 2 && BBLCorner(x,y) == 2)
                     return 0x0F;
                  else
                     return 0x92;
                  }
               else // BRCorner
                  {
                  if (BRRCorner(x,y) == 2 && BBRCorner(x,y) == 2)
                     return 0x0E;
                  else
                     return 0x8F;
                  }
               }
            
            // Straight edges
            
            // edge on bottom
            if (TLCorner(x,y) <  2 && TRCorner(x,y) <  2 && BLCorner(x,y) == 2 && BRCorner(x,y) == 2)
               {
               bool bLeftEased  = (BLLCorner(x,y) <  2 && BBLCorner(x,y) == 2);
               bool bRightEased = (BRRCorner(x,y) <  2 && BBRCorner(x,y) == 2);
               
               if ( bLeftEased &&  bRightEased)
                  return 0x17;
               if (!bLeftEased &&  bRightEased)
                  return 0x6C;
               if ( bLeftEased && !bRightEased)
                  return 0x6D;
               if (!bLeftEased && !bRightEased)
                  return (magic(x,y) & 0x01) ? 0xA3 : 0x6E;
               }
            // edge on top
            else if (TLCorner(x,y) == 2 && TRCorner(x,y) == 2 && BLCorner(x,y) <  2 && BRCorner(x,y) <  2)
               {
               bool bLeftEased  = (TLLCorner(x,y) <  2 && TTLCorner(x,y) == 2);
               bool bRightEased = (TRRCorner(x,y) <  2 && TTRCorner(x,y) == 2);
               
               if ( bLeftEased &&  bRightEased)
                  return 0x19;
               if (!bLeftEased &&  bRightEased)
                  return 0x6A;
               if ( bLeftEased && !bRightEased)
                  return 0x69;
               if (!bLeftEased && !bRightEased)
                  return (magic(x,y) & 0x01) ? 0xA5 : 0x6B;
               }
            // edge on right
            if (TLCorner(x,y) <  2 && BLCorner(x,y) <  2 && TRCorner(x,y) == 2 && BRCorner(x,y) == 2)
               {
               bool bTopEased = (TTRCorner(x,y) <  2 && TRRCorner(x,y) == 2);
               bool bBotEased = (BBRCorner(x,y) <  2 && BRRCorner(x,y) == 2);
               
               if ( bTopEased &&  bBotEased)
                  return 0x16;
               if (!bTopEased &&  bBotEased)
                  return 0x73;
               if ( bTopEased && !bBotEased)
                  return 0x72;
               if (!bTopEased && !bBotEased)
                  return (magic(x,y) & 0x01) ? 0xA2 : 0x74;
               }
            // edge on left
            if (TLCorner(x,y) == 2 && BLCorner(x,y) == 2 && TRCorner(x,y) <  2 && BRCorner(x,y) <  2)
               {
               bool bTopEased = (TTLCorner(x,y) <  2 && TLLCorner(x,y) == 2);
               bool bBotEased = (BBLCorner(x,y) <  2 && BLLCorner(x,y) == 2);
               
               if ( bTopEased &&  bBotEased)
                  return 0x18;
               if (!bTopEased &&  bBotEased)
                  return 0x6F;
               if ( bTopEased && !bBotEased)
                  return 0x70;
               if (!bTopEased && !bBotEased)
                  return (magic(x,y) & 0x01) ? 0xA4 : 0x71;
               }
            
            // edge on bottom
            if (TLCorner(x,y) == 1 && TRCorner(x,y) == 1 && BLCorner(x,y) == 2 && BRCorner(x,y) == 2)
               {
               // no other high corners
               if (BLLCorner(x,y) <  2 && BRRCorner(x,y) <  2)
                  return 0x17;
               // high corner on left
               if (BLLCorner(x,y) == 2 && BRRCorner(x,y) <  2)
                  return 0x6C;
               // high corner on right
               if (BLLCorner(x,y) <  2 && BRRCorner(x,y) == 2)
                  return 0x6D;
               // both neighbor corners high
               if (BLLCorner(x,y) == 2 && BRRCorner(x,y) == 2)
                  return (magic(x,y) & 0x01) ? 0xA3 : 0x6E;
               }
            // edge on top
            else if (TLCorner(x,y) == 2 && TRCorner(x,y) == 2 && BLCorner(x,y) == 1 && BRCorner(x,y) == 1)
               {
               // no other high corners
               if (TLLCorner(x,y) <  2 && TRRCorner(x,y) <  2)
                  return 0x19;
               // high corner on left
               if (TLLCorner(x,y) == 2 && TRRCorner(x,y) <  2)
                  return 0x6A;
               // high corner on right
               if (TLLCorner(x,y) <  2 && TRRCorner(x,y) == 2)
                  return 0x69;
               // both neighbor corners high
               if (TLLCorner(x,y) == 2 && TRRCorner(x,y) == 2)
                  return (magic(x,y) & 0x01) ? 0xA5 : 0x6B;
               }
            // edge on right
            if (TLCorner(x,y) == 1 && BLCorner(x,y) == 1 && TRCorner(x,y) == 2 && BRCorner(x,y) == 2)
               {
               // no high neighbor corners
               if (TTRCorner(x,y) <  2 && BBRCorner(x,y) <  2)
                  return 0x16;
               // high neighbor corner on top
               if (TTRCorner(x,y) == 2 && BBRCorner(x,y) <  2)
                  return 0x73;
               // high neighbor corner on bottom
               if (TTRCorner(x,y) <  2 && BBRCorner(x,y) == 2)
                  return 0x72;
               // both neighbor corners high
               if (TTRCorner(x,y) == 2 && BBRCorner(x,y) == 2)
                  return (magic(x,y) & 0x01) ? 0xA2 : 0x74;
               }
            // edge on left
            if (TLCorner(x,y) == 2 && BLCorner(x,y) == 2 && TRCorner(x,y) == 1 && BRCorner(x,y) == 1)
               {
               // no high neighbor corners
               if (TTLCorner(x,y) <  2 && BBLCorner(x,y) <  2)
                  return 0x18;
               // high neighbor corner on top
               if (TTLCorner(x,y) == 2 && BBLCorner(x,y) <  2)
                  return 0x6F;
               // high neighbor corner on bottom
               if (TTLCorner(x,y) <  2 && BBLCorner(x,y) == 2)
                  return 0x70;
               // both neighbor corners high
               if (TTLCorner(x,y) == 2 && BBLCorner(x,y) == 2)
                  return (magic(x,y) & 0x01) ? 0xA4 : 0x71;
               }
            
            // Three corner cases
            
            // Three corner cases
            
            // 0 1         1 1           0 0 0       0 0 0
            // 0 1 1 0x77  0 1 1 0x76    0 1 1 0x7A  0 1 1 0x78
            // 0 0 0       0 0 0         0 1         1 1  
            //
            // 0 1         1 1           0 0 1       0 0 1
            // 0 1 1 0x75  0 1 1 0x13    0 1 1 0x79  0 1 1 0x14
            // 0 0 1       0 0 1         0 1         1 1  
            //
            //
            // 0 0 0       1 0 0           1 0         1 1
            // 1 1 0 0x7D  1 1 0 0x7B    1 1 0 0x80  1 1 0 0x7E
            //   1 0         1 0         0 0 0       0 0 0
            //
            // 0 0 0       1 0 0           1 0         1 1
            // 1 1 0 0x7C  1 1 0 0x15    1 1 0 0x7F  1 1 0 0x12
            //   1 1         1 1         1 0 0       1 0 0
            
            // corner in upper left
            if (BLCorner(x,y) == 2 && TLCorner(x,y) == 2 && TRCorner(x,y) == 2)
               {
               bool BLDiag = (BLLCorner(x,y) > 1 && BBLCorner(x,y) < 2);
               bool TRDiag = (TTRCorner(x,y) > 1 && TRRCorner(x,y) < 2);
               
               if      (!BLDiag && !TRDiag)
                  return 0x7A;
               else if (!BLDiag &&  TRDiag)
                  return 0x79;
               else if ( BLDiag && !TRDiag)
                  return 0x78;
               else
                  return 0x14;
               }
            
            // corner in upper right
            if (TLCorner(x,y) == 2 && TRCorner(x,y) == 2 && BRCorner(x,y) == 2)
               {
               bool TLDiag = ((TTLCorner(x,y) > 1) && (TLLCorner(x,y) < 2));
               bool BRDiag = ((BRRCorner(x,y) > 1) && (BBRCorner(x,y) < 2));
               
               if      (!TLDiag && !BRDiag)
                  return 0x7D;
               else if (!TLDiag &&  BRDiag)
                  return 0x7C;
               else if ( TLDiag && !BRDiag)
                  return 0x7B;
               else
                  return 0x15;
               }
            
            // corner in bottom right
            if (TRCorner(x,y) == 2 && BRCorner(x,y) == 2 && BLCorner(x,y) == 2)
               {
               bool TRDiag = (TRRCorner(x,y) > 1 && TTRCorner(x,y) < 2);
               bool BLDiag = (BBLCorner(x,y) > 1 && BLLCorner(x,y) < 2);
               
               if      (!TRDiag && !BLDiag)
                  return 0x80;
               else if (!TRDiag &&  BLDiag)
                  return 0x7F;
               else if ( TRDiag && !BLDiag)
                  return 0x7E;
               else
                  return 0x12;
               }
            
            // corner in bottom left
            if (BRCorner(x,y) == 2 && BLCorner(x,y) == 2 && TLCorner(x,y) == 2)
               {
               bool TLDiag = (TLLCorner(x,y) > 1 && TTLCorner(x,y) < 2);
               bool BRDiag = (BBRCorner(x,y) > 1 && BRRCorner(x,y) < 2);
               
               if      (!TLDiag && !BRDiag)
                  return 0x77;
               else if (!TLDiag &&  BRDiag)
                  return 0x75;
               else if ( TLDiag && !BRDiag)
                  return 0x76;
               else
                  return 0x13;
               }
            
            // Opposing corner cases
            if (TLCorner(x,y) == 2 && BRCorner(x,y) == 2)
               {
               // There are four cases, big big, big small, small big, small small
               
               // big big
               if (TLLCorner(x,y) == 2 && TTLCorner(x,y) == 2 && BRRCorner(x,y) == 2 && BBRCorner(x,y) == 2)
                  return 0x4F;
               // big small
               if (TLLCorner(x,y) == 2 && TTLCorner(x,y) == 2)
                  return 0x8A;
               // small big
               if (BRRCorner(x,y) == 2 && BBRCorner(x,y) == 2)
                  return 0x8B;
               // small small
               return 0x8D;
               }
            if (TRCorner(x,y) == 2 && BLCorner(x,y) == 2)
               {
               // There are four cases, big big, big small, small big, small small
               
               // big big
               if (TRRCorner(x,y) == 2 && TTRCorner(x,y) == 2 && BLLCorner(x,y) == 2 && BBLCorner(x,y) == 2)
                  return 0x50;
               // big small
               if (TRRCorner(x,y) == 2 && TTRCorner(x,y) == 2)
                  return 0x8E;
               // small big
               if (BLLCorner(x,y) == 2 && BBLCorner(x,y) == 2)
                  return 0x90;
               // small small
               return 0x91;
               }
         }
         
         throw CMapError(x, y, "illegal corner height arrangement");
      }
      else if ('W' == ch)
         {
         // Check to make sure that we're on ground level
         if (TLCorner(x,y) > 0 || TRCorner(x,y) > 0 || BLCorner(x,y) > 0 || BRCorner(x,y) > 0)
            throw CMapError(x, y, "water must be on a flat tile");
         
         index = (('W' != TLCenter(x,y)) << 7) |
            (('W' != TCenter(x,y))  << 6) | 
            (('W' != TRCenter(x,y)) << 5) |
            (('W' != LCenter(x,y))  << 4) |
            (('W' != RCenter(x,y))  << 3) |
            (('W' != BLCenter(x,y)) << 2) |
            (('W' != BCenter(x,y))  << 1) |
            ('W' != BRCenter(x,y));
         
         WORD nWaterTile = aWaterTileMapG[index];
         
         if (0x44 == nWaterTile)
            {
            WORD aWaterBlanks[] = { 0x45, 0x46, 0x44, 0x47 };
            
            nWaterTile = aWaterBlanks[magic(x,y)];
            }
         
         return nWaterTile;
         }
      else
         throw CMapError(x, y, "illegal tile character");
      
   throw CMapError(x, y, "unknown tile find error");
   }