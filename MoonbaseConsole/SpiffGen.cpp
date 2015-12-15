//MBC mapper by William Myers

// Revision history:
// 08/08/2003 WNM Initial C++ version
// 08/08/2003 EJA Syntax cleanup and compilation
// 09/03/2003 WNM Rewritten, more start positions added.
// 10/05/2003 WNM Inconsistent result bug hopefully fixed, water input added,
//                pool number and size now random based on map size and energy input


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include "mbcmif.h"
#include "GameInfo.h"

#define MAXELEVVAL 4 //for array size
#define HIGH 3 //elevations
#define MEDIUM 2
#define LOW 1
#define WATER 0 //special types
#define HUB 1
#define SMALLPOOL 2
#define MEDIUMPOOL 3
#define LARGEPOOLTOP 4
#define LARGEPOOLBOTTOM 5 //placeholder for top half of a large pool

#define UNASSIGNED -1
#define LOW_OR_WATER -2

#define MAXSIZE 80

int numPoolsG; //per quadrant
int energyAmountG; //2048 = min energy on small map, 51200 = max energy on max map, etc.
int cliffAmountG; //amount of cliffs, 10 is min, 70 is max
int waterAmountG; //0 is min, 30 is max
int totalMapSizeG;

int terrainSeedFlagG; //disables HIGH or LOW terrain for the initial elevation when appropriate 
int islandsFlagG; //enables islands
int advancedMirrorOK_G; //low terrain roughness can leave too abrupt changes at the edge, so set false to disable some mirroring types
int mirrorTypeG; //what mirroring is used
#define NORMALMIRROR 0
#define XOFFSETMIRROR 1
#define YOFFSETMIRROR 2
#define MAXDISTMIRROR 3

int mapCornerMaxG; //size of random section
int mapMiddleMaxG;
int mapCorner[MAXSIZE+1][MAXSIZE+1];
int mapMiddle[MAXSIZE][MAXSIZE];

//void stats();

float float_rand()
   {
   // The rand function returns a pseudorandom integer in the range 0 to RAND_MAX.
   return ((float)rand()) / RAND_MAX;
   }

int SpiffRand(int min, int max) //returns a random integer min to max inclusive
   {
   return ((int)(float_rand() * (max + 1 - min))) + min;
   }

int pickFrom2(int A, int probA, int B, int probB)
   {
   float R = float_rand() * (probA + probB);
   if (R < probA)
      return A;
   else
      return B;
   }

int pickFrom3(int A, int probA, int B, int probB, int C, int probC)
   {
   float R = float_rand() * (probA + probB + probC);
   if (R < probA)
      return A;
   else if (R < probA + probB)
      return B;
   else
      return C;
   }

int pickFrom4(int A, int probA, int B, int probB, int C, int probC, int D, int probD)
   {
   float R = float_rand() * (probA + probB + probC + probD);
   if (R < probA)
      return A;
   else if (R < probA + probB)
      return B;
   else if (R < probA + probB + probC)
      return C;
   else
      return D;
   }

void getSpecials() //choose where the starting points and pools are
   {
   int x, y, p, t;
   int edgeWaterA = (int)(islandsFlagG * totalMapSizeG / 16 + 0.5);
   int edgeWaterB = (int)(islandsFlagG * totalMapSizeG / 16); //don't put pools between islands

   // No matter what, they get a start hub spot.
   if (mirrorTypeG == MAXDISTMIRROR)
      x = (int)((totalMapSizeG*3+8)/16);
   else
      x = (int)(mapMiddleMaxG/2);
   y = x;

   mapMiddle[x][y] = HUB; //hub start position

   for (p = 1; p <= numPoolsG; ++p)
      {
      x = SpiffRand(edgeWaterA, mapMiddleMaxG-edgeWaterB);
      y = SpiffRand(edgeWaterA, mapMiddleMaxG-edgeWaterB);
      if (mapMiddle[x][y] != UNASSIGNED)
         --p;//repick this pool
      else
         {
         t = pickFrom3(SMALLPOOL,40000*numPoolsG, MEDIUMPOOL,20000*numPoolsG+energyAmountG, LARGEPOOLTOP,2*energyAmountG);
         if (t == LARGEPOOLTOP)
            if ((y == mapMiddleMaxG-edgeWaterB)||(mapMiddle[x][y+1] != UNASSIGNED))
               t = SMALLPOOL; //keep large pool from being too high or overlapping another pool or start
            else
               mapMiddle[x][y+1] = LARGEPOOLBOTTOM;
         mapMiddle[x][y] = t;
         }
      }
   }
void copyMap(int XOffset, int YOffset, int XDirection, int YDirection)
//copies the first quadrant of the map
//XOffset and YOffset are the distances moved
//XDirection and/or YDirection are/is -1 for mirrored, 1 for not mirrored
   {
   int x;
   int y;
   int tempMiddle;
   int newCX;
   int newCY = YOffset;
   int newMX;
   int newMY;
   if (YDirection < 0)
      newCY += mapCornerMaxG;

   for (y = 0; y <= mapCornerMaxG; ++y)
      {
      if (newCY < 0)
         newCY += totalMapSizeG;
      else if (newCY >= totalMapSizeG)
         newCY -= totalMapSizeG;

      newCX = XOffset;
      if (XDirection < 0)
         newCX += mapCornerMaxG;

      for (x = 0; x <= mapCornerMaxG; ++x)
         {
         if (newCX < 0)
            newCX += totalMapSizeG;
         else if (newCX >= totalMapSizeG)
            newCX -= totalMapSizeG;

         mapCorner[newCX][newCY] = mapCorner[x][y];
         if ((x != mapCornerMaxG)&&(y != mapCornerMaxG))
            {
            tempMiddle = mapMiddle[x][y];
            newMX = newCX;
            newMY = newCY;
            if (YDirection < 0)
               {
               newMY--;
               if (newMY == -1)
                  newMY = totalMapSizeG - 1;
               if (tempMiddle == LARGEPOOLTOP)
                  tempMiddle = LARGEPOOLBOTTOM;
               else if (tempMiddle == LARGEPOOLBOTTOM)
                  tempMiddle = LARGEPOOLTOP;
               }
            if (XDirection < 0)
               {
               newMX--;
               if (newMX == -1)
                  newMX = totalMapSizeG - 1;
               }
               mapMiddle[newMX][newMY] = tempMiddle;
            }

         newCX += XDirection;
         }

      newCY += YDirection;
      }
   }

void mirrorMap()
   {
//--------------------------------------------------------------
// mirror map
//--------------------------------------------------------------
//     normalMirroring()

   int swapXa = pickFrom2(-1,1, 1,advancedMirrorOK_G);
   int swapYa = pickFrom2(-1,advancedMirrorOK_G, 1,1);
   int swapXb = pickFrom2(-1,advancedMirrorOK_G, 1,1);
   int swapYb = pickFrom2(-1,1, 1,advancedMirrorOK_G);

   switch (mirrorTypeG)
      {
      case NORMALMIRROR: //four quadrants
         //ABCBA
         //DEFED
         //GHIHG
         //DEFED
         //ABCBA
         copyMap(mapCornerMaxG, 0, swapXa, swapYa);
         copyMap(0, mapCornerMaxG, swapXb, swapYb);
         copyMap(mapCornerMaxG, mapCornerMaxG, swapXa*swapXb, swapYa*swapYb);
         break;
      case XOFFSETMIRROR: //Like normal, but one half is moved horizontally by 1/4 totalmapsize
         //ABABABABA
         //DEFGHGFED
         //CDCDCDCDC
         //FGHGFEDEF
         //ABABABABA
         if (swapYa == -1) //ensures fairness
            swapXb = -1;
         copyMap(mapCornerMaxG, 0, 1, swapYa);
         copyMap(mapCornerMaxG/2, mapCornerMaxG, swapXb, swapYb);
         copyMap(mapCornerMaxG*3/2, mapCornerMaxG, swapXb, swapYa*swapYb);
         break;
      case YOFFSETMIRROR: //Like normal, but one half is moved vertically by 1/4 totalmapsize
         if (swapXb == -1) //ensures fairness
            swapYa = -1;
         copyMap(mapCornerMaxG, mapCornerMaxG/2, swapXa, swapYa);
         copyMap(0, mapCornerMaxG, swapXb, 1);
         copyMap(mapCornerMaxG, mapCornerMaxG*3/2, swapXa*swapXb, swapYa);
         break;
      case MAXDISTMIRROR: //Allows maximum distance between starting points
      default:
         //ABCDCBA
         //E*GHIJE
         //HIJE*GH
         //DCBABCD
         //HG*EJIH
         //EJIHG*E
         //ABCDCBA

         copyMap(mapCornerMaxG, 0, 1, -1);
         copyMap(0, mapCornerMaxG, -1, 1);
         copyMap(mapCornerMaxG, mapCornerMaxG, -1, -1);
      }
   }

void errorCorrection()
//corrects errors caused by pool placement and mirroring
//doesn't correct mapCorner[x][totalMapSizeG+1] or mapCorner[totalMapSizeG+1][y], since it isn't used

//for any HIGH to LOW transitions, makes the HIGH MEDIUM
//for pools on nonflat terrain, makes the terrain MEDIUM
//removes invalid water

   {
   int x;
   int y;
   int tempX;
   int tempY;
   int dx;
   int dy;
   int redo;
   int elev;

   for (y = 0; y < totalMapSizeG; ++y)
      for (x = 0; x < totalMapSizeG; ++x)
         if (mapCorner[x][y] == HIGH)
            for (dy = -1; dy <= 1; ++dy)
               {
               tempY = y + dy;
               if (tempY == totalMapSizeG)
                  tempY = 0;
               else if (tempY == -1)
                  tempY = totalMapSizeG-1;

               for (dx = -1; dx <= 1; ++dx)
                  {
                  tempX = x + dx;
                  if (tempX == totalMapSizeG)
                     tempX = 0;
                  else if (tempX == -1)
                     tempX = totalMapSizeG-1;
                  if (mapCorner[tempX][tempY] == LOW)
                     mapCorner[x][y] = MEDIUM;
                  }
               }
         else if ((mapCorner[x][y] != LOW)&&(mapCorner[x][y] != MEDIUM))
            mapCorner[x][y] = MEDIUM; //should not happen anymore

   do
      {
      redo = 0;
      for (y = 0; y < totalMapSizeG; ++y)
         for (x = 0; x < totalMapSizeG; ++x)
            if (mapMiddle[x][y] != UNASSIGNED)
               {
               tempY = y + 1;
               if (tempY == totalMapSizeG)
                  tempY = 0;

               tempX = x + 1;
               if (tempX == totalMapSizeG)
                  tempX = 0;

               elev = mapCorner[x][y];
               if ((mapMiddle[x][y] == WATER)&&(elev != LOW))
                  mapMiddle[x][y] = UNASSIGNED;
               else if ((elev != mapCorner[x][tempY])||(elev != mapCorner[tempX][y])||(elev != mapCorner[tempX][tempY]))
                  if (mapMiddle[x][y] == WATER)
                     mapMiddle[x][y] = UNASSIGNED;
                  else
                     {
                     mapCorner[x][y] = MEDIUM;
                     mapCorner[x][tempY] = MEDIUM;
                     mapCorner[tempX][y] = MEDIUM;
                     mapCorner[tempX][tempY] = MEDIUM;
                     redo = 1;
                     }
               }
      }
   while (redo); //changing a pool may require others to change (this is the easiest way, not the most efficient)
   }

void generate()
   {
//--------------------------------------------------------------
//initialize
//--------------------------------------------------------------
   int x;
   int y;
   int neighbors[MAXELEVVAL];
   int a;
   int b;
   int tempElevation;
   int nextElevation;
   int special;

   mapCornerMaxG = totalMapSizeG/2;
   mapMiddleMaxG = mapCornerMaxG - 1;

   for (y = 0; y <= mapCornerMaxG; ++y) //initialise map to UNASSIGNED tiles
      for (x = 0; x <= mapCornerMaxG; ++x)
         {
         mapCorner[x][y] = UNASSIGNED;
         mapMiddle[x][y] = UNASSIGNED;
         }
   if (advancedMirrorOK_G)
      mirrorTypeG = pickFrom4(NORMALMIRROR,1, XOFFSETMIRROR,2, YOFFSETMIRROR,2, MAXDISTMIRROR, 4);
   else
      mirrorTypeG = NORMALMIRROR;
   getSpecials(); //get start and pools

//--------------------------------------------------------------
// loop through each square
//--------------------------------------------------------------
   mapCorner[0][0] = pickFrom3(LOW,1, MEDIUM,(terrainSeedFlagG < 9), HIGH,(terrainSeedFlagG < 8)); //seed
//   mapCorner[0][0] = LOW;//HIGH;//MEDIUM;//
   for (y = 0; y <= mapCornerMaxG; ++y) //fill in the rest of the random map
      for (x = 0; x <= mapCornerMaxG; ++x)
         {
         special = mapMiddle[x][y]; //water wouldn't have been assigned yet, so must be pool, start, or UNASSIGNED

//--------------------------------------------------------------
// check neighbors
//--------------------------------------------------------------
         if ((mapCorner[x][y] != UNASSIGNED)&&(mapCorner[x][y] != LOW_OR_WATER))
            nextElevation = mapCorner[x][y]; //already defined because of a special or (0,0), so no change
         else
            {
            neighbors[HIGH] = 0;
            neighbors[MEDIUM] = 0;
            neighbors[LOW] = 0;
            neighbors[WATER] = 0;

            if (x > 0)
               {
               a = mapCorner[x-1][y];
               if((y > 1)&&(mapMiddle[x-1][y-2] == WATER))
                  ++neighbors[WATER];
               if (y > 0)
                  neighbors[mapCorner[x-1][y-1]] += 3;
               }
            else
               a = mapCorner[x][y-1];
//               {
//               a = mapCorner[mapCornerMaxG][y-1];
//               if((y > 1)&&(mapMiddle[mapCornerMaxG][y-2] == WATER))
//                  ++neighbors[WATER];
//               }
            neighbors[a] += 3;
            if (y > 0)
               {
               b = mapCorner[x][y-1];
               neighbors[b] += 3;
               if (x < mapCornerMaxG)
                  {
                  ++neighbors[mapCorner[x+1][y-1]]; //so this value can be ignored when choosing water
                  if ((special != UNASSIGNED)&&(x < mapCornerMaxG-1))
                     ++neighbors[mapCorner[x+2][y-1]];
//                  else
//                     ++neighbors[mapCorner[0][y-1]];
                  }
//               else
//                  ++neighbors[mapCorner[0][y-1]];
               if((x > 1)&&(mapMiddle[x-2][y-1] == WATER))
                  ++neighbors[WATER];
               }
            else
               b = mapCorner[x-1][y]; // for probability equations for edges

//--------------------------------------------------------------
// pick new elevation
//--------------------------------------------------------------
            //neighbors                  possible new elevation
            //HIGH or HIGH with MEDIUM   HIGH or MEDIUM
            //MEDIUM only                HIGH, MEDIUM or LOW
            //LOW or WATER only          MEDIUM, LOW or WATER
            //MEDIUM with LOW or WATER   MEDIUM or LOW, possible WATER if no MEDIUM left, down, or down-left
            //HIGH with LOW or WATER     MEDIUM
#define HIGH_AMT 105 //default elevation distributions
#define MEDIUM_AMT (100+waterAmountG)
#define LOW_AMT (105+3*waterAmountG)
#define WATER_AMT 15*waterAmountG

            if (neighbors[LOW])
               if (neighbors[HIGH]) //HIGH with LOW or WATER
                  nextElevation = MEDIUM;
               else if (neighbors[MEDIUM] >= 3) //MEDIUM with LOW or WATER
                  if (a != b)
                     nextElevation = pickFrom2(LOW,LOW_AMT, MEDIUM,MEDIUM_AMT);
                  else if (a == LOW)
                     nextElevation = pickFrom2(LOW,100*LOW_AMT, MEDIUM,MEDIUM_AMT*cliffAmountG);
                  else
                     nextElevation = pickFrom2(LOW,LOW_AMT*cliffAmountG, MEDIUM,100*MEDIUM_AMT);
               else //LOW or WATER only, possibly MEDIUM down-right
                  if (neighbors[WATER] == 1)
                     nextElevation = pickFrom3(WATER,100*WATER_AMT, LOW,100*LOW_AMT, MEDIUM,MEDIUM_AMT*cliffAmountG);
                  else if (neighbors[WATER] == 0)
                     nextElevation = pickFrom3(WATER,WATER_AMT*cliffAmountG, LOW,100*LOW_AMT, MEDIUM,MEDIUM_AMT*cliffAmountG);
                  else
                     nextElevation = pickFrom3(WATER,10000*WATER_AMT, LOW,LOW_AMT*100*cliffAmountG, MEDIUM,MEDIUM_AMT*cliffAmountG*cliffAmountG);
            else
               if (neighbors[HIGH]) //HIGH or HIGH with MEDIUM
                  if (a != b)
                     nextElevation = pickFrom2(MEDIUM,MEDIUM_AMT,HIGH,HIGH_AMT);
                  else if (a == HIGH)
                     nextElevation = pickFrom2(MEDIUM,MEDIUM_AMT*cliffAmountG,HIGH,100*HIGH_AMT);
                  else
                     nextElevation = pickFrom2(MEDIUM,100*MEDIUM_AMT,HIGH,HIGH_AMT*cliffAmountG);
               else //MEDIUM only
                  nextElevation = pickFrom3(LOW,LOW_AMT*cliffAmountG,MEDIUM,200*MEDIUM_AMT, HIGH,HIGH_AMT*cliffAmountG);

//--------------------------------------------------------------
// set elevation
//--------------------------------------------------------------
            if ((mapCorner[x][y] == LOW_OR_WATER)&&(nextElevation != WATER))
//bottom and left edges of a special on LOW ground there may only be LOW or WATER
               nextElevation = LOW;

            if (nextElevation == WATER)
               {
               if ((x != 0)&&(y != 0)&&(mapMiddle[x-1][y-1] == UNASSIGNED))
                  mapMiddle[x-1][y-1] = WATER; //set WATER
               nextElevation = LOW;
               }
            }


         mapCorner[x][y] = nextElevation; //set elevation

         if (special != UNASSIGNED) //if special, make flat spot (don't worry about going over map edge, will go into mirrored part)
            {
            tempElevation = nextElevation;
            if(tempElevation == LOW)
               tempElevation = LOW_OR_WATER; //allow for water on left and bottom edges
            mapCorner[x+1][y+1] = nextElevation;
            mapCorner[x+1][y] = tempElevation;
            mapCorner[x][y+1] = tempElevation;
            }
         }

   if (islandsFlagG) //replace borders with water, errorCorrection() finishes it.
      {
      int edgeWaterA = (int)(islandsFlagG * totalMapSizeG / 16 + 0.5);
      int edgeWaterB = mapMiddleMaxG-(int)(islandsFlagG * totalMapSizeG / 16);
      for (y = 0; y <= mapCornerMaxG; ++y)
         {
         for (x = 0; x < edgeWaterA; ++x)
            {
            mapCorner[x][y] = LOW;
            mapMiddle[x][y] = WATER;
            }
         if(mapCorner[edgeWaterA+1][y] == HIGH)
            mapCorner[edgeWaterA][y] = MEDIUM;

         for (x = mapMiddleMaxG; x > edgeWaterB; --x)
            {
            mapCorner[x+1][y] = LOW;
            mapMiddle[x][y] = WATER;
            }
         if(mapCorner[edgeWaterB][y] == HIGH)
            mapCorner[edgeWaterB+1][y] = MEDIUM;
         }

      for (x = edgeWaterA; x <= edgeWaterB+1; ++x)
         {
         for (y = 0; y < edgeWaterA; ++y)
            {
            mapCorner[x][y] = LOW;
            mapMiddle[x][y] = WATER;
            }
         if(mapCorner[x][edgeWaterA+1] == HIGH)
            mapCorner[x][edgeWaterA] = MEDIUM;

         for (y = mapMiddleMaxG; y > edgeWaterB; --y)
            {
            mapCorner[x][y+1] = LOW;
            mapMiddle[x][y] = WATER;
            }
         if(mapCorner[x][edgeWaterB] == HIGH)
            mapCorner[x][edgeWaterB+1] = MEDIUM;
         }
      if (islandsFlagG == 2) //add tiny islands to help bridge wide channels
         {
         int j;
         for (int i = 0; i < totalMapSizeG / 16; ++i)
            {
            x = (int)(totalMapSizeG / 16 - .5);//SpiffRand((int)(totalMapSizeG / 16 - .5), (int)(totalMapSizeG / 8 - 3));
            y = SpiffRand(x, totalMapSizeG/2 - 1 - x);//(int)(totalMapSizeG / 16 - .5), totalMapSizeG/2-(int)(totalMapSizeG / 16 - .5));
            if (SpiffRand(0,1))
               x = totalMapSizeG / 2 - 1 - x;
            if (SpiffRand(0,1))
               {
               mapMiddle[x][y] = UNASSIGNED;
               for (j = 0; j < 4; ++j)
                  mapMiddle[x + SpiffRand(-1,1)][y + SpiffRand(-1,1)] = UNASSIGNED;
               }
            else
               {
               mapMiddle[y][x] = UNASSIGNED;
               for (j = 0; j < 4; ++j)
                  mapMiddle[y + SpiffRand(-1,1)][x + SpiffRand(-1,1)] = UNASSIGNED;
               }
            }
         }
      }

   mirrorMap();
//   stats();
   errorCorrection();
   }
/*
int totalTransitions = 0;
int totalWaterTransitions = 0;
int totalHigh = 0;
int totalMedium = 0;
int totalLow = 0;
int totalWater = 0;
int totalSPools = 0;
int totalMPools = 0;
int totalLPools = 0;

void stats()
   {
   int y;
   int x;
   int t;
   int LevelMap[MAXELEVVAL];
   int SPools = 0;
   int MPools = 0;
   int LPools = 0;
   int Transitions = 0;
   int WaterTransitions = 0;

   LevelMap[HIGH] = 0;
   LevelMap[MEDIUM] = 0;
   LevelMap[LOW] = 0;
   LevelMap[WATER] = 0;

   for (y = 0; y < totalMapSizeG; ++y)
      for (x = 0; x < totalMapSizeG; ++x)
         {
         t = mapCorner[x][y];
         ++LevelMap[t];
         if (t != mapCorner[(x+1)%totalMapSizeG][y])
            ++Transitions;
         if (t != mapCorner[x][(y+1)%totalMapSizeG])
            ++Transitions;
         switch (mapMiddle[x][y])
            {
            case SMALLPOOL:
               ++SPools;
               break;
            case MEDIUMPOOL:
               ++MPools;
               break;
            case LARGEPOOLTOP:
               ++LPools;
               break;
            case WATER:
               ++LevelMap[WATER];
               if (t != mapMiddle[(x+1)%totalMapSizeG][y])
                  ++WaterTransitions;
               if (t != mapMiddle[x][(y+1)%totalMapSizeG])
                  ++WaterTransitions;

            }
         }
   LevelMap[LOW] -= LevelMap[WATER];

   cout << "\nH: ";
   cout << LevelMap[HIGH];
   cout << "\tM: ";
   cout << LevelMap[MEDIUM];
   cout << "\tL: ";
   cout << LevelMap[LOW];
   cout << "\tW: ";
   cout << LevelMap[WATER];
   cout << "\tSP: ";
   cout << SPools;
   cout << "\tMP: ";
   cout << MPools;
   cout << "\tLP: ";
   cout << LPools;
   cout << "\tT: ";
   cout << Transitions;
   cout << "\tWT: ";
   cout << WaterTransitions;

   totalWaterTransitions += WaterTransitions;
   totalTransitions += Transitions;
   totalHigh += LevelMap[HIGH];
   totalMedium += LevelMap[MEDIUM];
   totalLow += LevelMap[LOW];
   totalWater += LevelMap[WATER];
   totalSPools += SPools;
   totalMPools += MPools;
   totalLPools += LPools;
   }

void makeMYCROFTfile()
   {
   char charOut[MAXELEVVAL];

   charOut[HIGH] = '2';
   charOut[MEDIUM] = '1';
   charOut[LOW] = '0';

   cout << "MBCMIF1\n";
   switch (totalMapSizeG)
      {
      case 32:
         cout << "SMALL\n";
         break;
      case 40:
         cout << "MEDIUM\n";
         break;
      case 48:
         cout << "LARGE\n";
         break;
      case 56:
         cout << "HUGE\n";
         break;
      case 64:
         cout << "SAI\n";
         break;
      case 72:
         cout << "OBSCENE\n";
         break;
      case 80:
         cout << "MAX\n";
         break;
      default:
         cout << "?????\n";
      }

   int y;
   int x;

   for (y = 0; y < totalMapSizeG; ++y)
      {
      for (x = 0; x < totalMapSizeG; ++x)
         {
         cout << charOut[mapCorner[x][y]];
         cout <<  ' ';
         }
      cout << "\n";
      for (x = 0; x < totalMapSizeG; ++x)
         switch (mapMiddle[x][y])
            {
            case HUB:
               cout << "01";
               break;
            case SMALLPOOL:
               cout << " S";
               break;
            case MEDIUMPOOL:
               cout << " M";
               break;
            case LARGEPOOLTOP:
               cout << " L";
               break;
            case WATER:
               cout << " W";
               break;
            case UNASSIGNED:
            case LARGEPOOLBOTTOM:
               cout << " .";
               break;
            default:
               cout << "??";
            }
      cout << "\n";
      }
   }
*/
static void PopulateMIF(MBCMIF &mif)
   {
   int LevelMap[MAXELEVVAL];

   LevelMap[HIGH] = 2;
   LevelMap[MEDIUM] = 1;
   LevelMap[LOW] = 0;

   mif.nVersion = 3;
   mif.nDimension = totalMapSizeG;

   int y;
   int x;
   char t;
   int XOffset = SpiffRand(0, totalMapSizeG-1);
   int YOffset = SpiffRand(0, totalMapSizeG-1);
   int newX;
   int newY;

   for (y = 0, newY = YOffset; y < totalMapSizeG; ++y, ++newY)
      for (x = 0, newX = XOffset; x < totalMapSizeG; ++x, ++newX)
         {
         if (newX == totalMapSizeG)
            newX = 0;
         if (newY == totalMapSizeG)
            newY = 0;
         mif.aaCornerMap[newX][newY] = LevelMap[mapCorner[x][y]];
         switch (mapMiddle[x][y])
            {
            case HUB:
               t = -1;
               break;
            case SMALLPOOL:
               t = 'S';
               break;
            case MEDIUMPOOL:
               t = 'M';
               break;
            case LARGEPOOLBOTTOM:
               t = 'L';
               break;
            case WATER:
               t = 'W';
               break;
            case UNASSIGNED:
            case LARGEPOOLTOP:
               t = '.';
               break;
            default:
               t = '?';
            }
         mif.aaCenterMap[newX][newY] = t;
         }
   }

void SpiffGenerate (CGameInfo& gi, MBCMIF& mif)
   {
   int n;
   int waterInput = gi.nWater;

   totalMapSizeG = gi.nMapSize;
   energyAmountG = (2 + gi.nEnergy) * totalMapSizeG * totalMapSizeG;

   islandsFlagG = pickFrom2(0,1, gi.nWater-4,(gi.nWater >= 5)); //1 is large islands, 2 is small
   if (islandsFlagG)
      {
      waterInput -= 3;
      energyAmountG = (int)(energyAmountG * (5-islandsFlagG) / 6); // *2/3 or *1/2
      }

   waterAmountG = 4 * waterInput;
   cliffAmountG = 1 << gi.nTerrain;//(int)((1 + gi.nTerrain) * (1 + gi.nTerrain) * 3/2);
   advancedMirrorOK_G = ((gi.nTerrain > 1) && (waterInput < 6)) || islandsFlagG;
   terrainSeedFlagG = 2*waterInput - gi.nTerrain;

   n = (int)(energyAmountG/2700);
   if (n > 12)
      n = 12;
   if (n < 1)
      n = 1;
   numPoolsG = SpiffRand((int)(energyAmountG/4000)+1, n);
   if (numPoolsG > 12)
      numPoolsG = 12;

   mif.nTileset = gi.nTileset;
   sprintf(mif.szName, "Spiff %04X", gi.nSeed);

   generate();

//   stats();
   PopulateMIF(mif);
//   makeMYCROFTfile();
   }
/*
void SpiffGenerate100 (CGameInfo& gi, MBCMIF& mif)
   {
   for (int x = 0; x < 100; ++x)
      SpiffGenerate(gi, mif);

   cout << "\ntotal H: ";
   cout << totalHigh;
   cout << "\tM: ";
   cout << totalMedium;
   cout << "\tL: ";
   cout << totalLow;
   cout << "\tW: ";
   cout << totalWater;
   cout << "\tSP: ";
   cout << totalSPools;
   cout << "\tMP: ";
   cout << totalMPools;
   cout << "\tLP: ";
   cout << totalLPools;
   cout << "\tT: ";
   cout << totalTransitions;
   cout << "\tWT: ";
   cout << totalWaterTransitions;

   PopulateMIF(mif);
   }
*/

