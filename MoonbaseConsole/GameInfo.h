#ifndef __GAMEINFO_H__
#define __GAMEINFO_H__

#define SPIFF_GEN  1
#define KATTON_GEN 2

#pragma pack(push, 1)
struct CGameInfo
   {
   unsigned int nMapSize   : 7;
   unsigned int nGenerator : 3;
   unsigned int nVersion   : 3;
   unsigned int nTileset   : 3;
   unsigned int nTerrain   : 4;
   unsigned int nWater     : 4;
   unsigned int nEnergy    : 4;
   unsigned short nSeed;
   unsigned char nIP1;
   unsigned char nIP2;
   unsigned char nIP3;
   unsigned char nIP4;
/*   union
      {
      unsigned long blanker;
      struct
         {
         BYTE cSpecials;
         BYTE nCliff;
         } SpiffData;
      struct
         {
         BYTE nSplotches;
         BYTE nLandFatness;
         BYTE nWater;
         BYTE nEnergy;
         } KattonData;
      } GeneratorData;*/
   };
#pragma pack(pop)

#endif