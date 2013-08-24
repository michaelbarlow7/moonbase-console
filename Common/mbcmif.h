#ifndef __MBCMIF_H__
#define __MBCMIF_H__



#include "MBCMapImage.h"
#include <stdio.h>



struct MBCMIF
   {
   int nVersion;
   int nDimension; // 32 (small), 40 (medium), 48 (large), 56 (huge), 64 (SAI)
   int nTileset;
   char szName[17];
   BYTE aaCornerMap[MAX_TILE_COUNT][MAX_TILE_COUNT];
   char aaCenterMap[MAX_TILE_COUNT][MAX_TILE_COUNT];

   MBCMIF ();
   void GenerateMap(MBCMapImage& mi);
   //void BuildCalibrationMap(void);
#ifndef MOONBASECONSOLE
   void Read_MIF_File (FILE *pf);
#endif
   void DefineStartLocations (MBCMapImage& mi);
   void DefineEnergyPools (MBCMapImage& mi);
   void MakeCraters(MBCMapImage& mi);
   WORD FindTileFor (int x, int y);

   inline char TLCenter (int x, int y) { return aaCenterMap[(0 == x) ? nDimension - 1 : x - 1][(0 == y) ? nDimension - 1 : y - 1]; }
   inline char TCenter (int x, int y)  { return aaCenterMap[x]                                [(0 == y) ? nDimension - 1 : y - 1]; }
   inline char TRCenter (int x, int y) { return aaCenterMap[(x + 1) % nDimension]             [(0 == y) ? nDimension - 1 : y - 1]; }
   inline char LCenter (int x, int y)  { return aaCenterMap[(0 == x) ? nDimension - 1 : x - 1][y]; }
   inline char RCenter (int x, int y)  { return aaCenterMap[(x + 1) % nDimension]             [y]; }
   inline char BLCenter (int x, int y) { return aaCenterMap[(0 == x) ? nDimension - 1 : x - 1][(y + 1) % nDimension]; }
   inline char BCenter (int x, int y)  { return aaCenterMap[x]                                [(y + 1) % nDimension]; }
   inline char BRCenter (int x, int y) { return aaCenterMap[(x + 1) % nDimension]             [(y + 1) % nDimension]; }

   inline BYTE TLCorner (int x, int y)   { return aaCornerMap[x][y]; }
   inline BYTE TRCorner (int x, int y)   { return aaCornerMap[(x + 1) % nDimension][y]; }
   inline BYTE BLCorner (int x, int y)   { return aaCornerMap[x][(y + 1) % nDimension]; }
   inline BYTE BRCorner (int x, int y)   { return aaCornerMap[(x + 1) % nDimension][(y + 1) % nDimension]; }

   inline BYTE TTLLCorner (int x, int y) { return TLCorner((x == 0) ? nDimension - 1 : x - 1, (y == 0) ? nDimension - 1: y - 1); }
   inline BYTE TTLCorner (int x, int y)  { return TRCorner((x == 0) ? nDimension - 1 : x - 1, (y == 0) ? nDimension - 1: y - 1); }
   inline BYTE TTRCorner (int x, int y)  { return TLCorner((x + 1) % nDimension,              (y == 0) ? nDimension - 1: y - 1); }
   inline BYTE TTRRCorner (int x, int y) { return TRCorner((x + 1) % nDimension,              (y == 0) ? nDimension - 1: y - 1); }
   inline BYTE TLLCorner (int x, int y)  { return TLCorner((x == 0) ? nDimension - 1 : x - 1, y); }
   inline BYTE TRRCorner (int x, int y)  { return TRCorner((x + 1) % nDimension,              y); }
   inline BYTE BLLCorner (int x, int y)  { return BLCorner((x == 0) ? nDimension - 1 : x - 1, y); }
   inline BYTE BRRCorner (int x, int y)  { return BRCorner((x + 1) % nDimension,              y); }
   inline BYTE BBLLCorner (int x, int y) { return BLCorner((x == 0) ? nDimension - 1 : x - 1, (y + 1) % nDimension); }
   inline BYTE BBLCorner (int x, int y)  { return BRCorner((x == 0) ? nDimension - 1 : x - 1, (y + 1) % nDimension); }
   inline BYTE BBRCorner (int x, int y)  { return BLCorner((x + 1) % nDimension,              (y + 1) % nDimension); }
   inline BYTE BBRRCorner (int x, int y) { return BRCorner((x + 1) % nDimension,              (y + 1) % nDimension); }
   };



#endif