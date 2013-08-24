#ifndef __MBCMAPIMAGE_H__
#define __MBCMAPIMAGE_H__



#include "globals.h"
#include <string.h>



#pragma pack(push, 1)
typedef struct tagPixelLoc
   {
   WORD xLoc;
   WORD yLoc;
   } PixelLoc;
typedef struct tagEnergyPoolLoc
   {
   PixelLoc plLocation;
   PixelLoc plDummy;
   } EnergyPoolLoc;
struct MBCMapImage
   {
   WORD wHeaderDummy;
   WORD cTilesX;
   WORD cTilesY;
   WORD nTileset;
   WORD cEnergyPools;
   WORD aaTiles[80][161];
   BYTE aSpace1[230];
   char szName[17];
   BYTE aSpace2[25837];
   EnergyPoolLoc aepLocs[49];
   PixelLoc epLastPool;
   WORD wDummy;
   PixelLoc apl4PLocs[4];     // 2^0
   PixelLoc apl3PLocs[3];     // 2^1
   PixelLoc apl2PLocs[2];     // 2^2
   PixelLoc apl2v2PLocs[4];   // 2^3
   PixelLoc apl1v3PLocs[4];   // 2^4
   PixelLoc apl1v2PLocs[3];   // 2^5

   MBCMapImage()
      {
      memset(this, 0, sizeof(MBCMapImage));

      nTileset = 1;
      memset((void *)apl4PLocs, 0xFF, sizeof(PixelLoc) * 20);
      szName[0] = 'N';
      }
   };
#pragma pack(pop)



#endif