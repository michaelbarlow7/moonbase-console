/*MBC Random Map Generator by Katton*/

#define VERSIONNUM 3.0
#define TEMP_REPLACEMENT_NUMA 5
#define TEMP_REPLACEMENT_NUMB 6
#define TEMP_REPLACEMENT_NUMC 7
#define NEVER_USED_NUM 99

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "mbcmif.h"
#include "GameInfo.h"

int size; // 32, 40, 48, or 56
int tileset;
int startloc [20][2];
int board [MAX_TILE_COUNT][MAX_TILE_COUNT];
int special [MAX_TILE_COUNT][MAX_TILE_COUNT];
char mapname[17];


int min (int a, int b, int c);
int distance (int x1, int y1, int x2, int y2);
int plusminus (int max);
int fillboards (int num);
int randomplace (int numberofplaces, int placer);
int randomflip (int numberofplaces, int inWater);
int randomsplotch(int length, int stringiness, int placer, int x, int y); //stringiness: 0 = no change/random, 1 = no back, 2 = back goes forwards
int goodforwater (int x, int y);
int randomwater(int length, int stringiness, int x, int y);
int goodforenergy (int x, int y, int poolsize);
int makeenergy (void);
int findcoord (int value, int move);
int replacenum (int replacee, int replacer);
int fattenone (int x, int y, int howfat, int middle, int ignorer, int replacer);
int fattenall (int howfat, int middle, int ignorer, int replacer); // howfat: postive 1-5 for distance, -100 to 0 for random 3 spread from 2 to 5.
int findstartloc (void);
int whatheightstartloc (int x, int y);
int tileaverage (int x, int y, int threshold);
int saveboard (void);
void KattonGenerate (void);

static void PopulateMIF (MBCMIF& mif)
{
	int i, j;
	mif.nVersion = 3;
	mif.nDimension = size;
	mif.nTileset = tileset;
	strcpy (mif.szName, mapname);
	for (j=0; j<size; j++)
	{
		for (i=0; i < size; i++)
			mif.aaCornerMap[i][j] = board[i][j];
		for (i=0; i < size; i++)
			if (special[i][j] == 0)
				mif.aaCenterMap[i][j] = '.';
			else if (special[i][j] == -1)
				mif.aaCenterMap[i][j] = 'W';
			else if (special[i][j] == 100)
				mif.aaCenterMap[i][j] = 'S';
			else if (special[i][j] == 200)
				mif.aaCenterMap[i][j] = 'M';
			else if (special[i][j] == 300)
				mif.aaCenterMap[i][j] = 'L';
			else
				mif.aaCenterMap[i][j] = -special[i][j];
	}
}
void KattonGenerate (CGameInfo& gi, MBCMIF& mif) {
	
	//****************************************Get values from GI Joe, the Great American Action Hero!
	
	sprintf(mapname, "Katton %04X", gi.nSeed);		// 0 <= x <= 10^5 - 1
	size = gi.nMapSize;								// 10 <= x <= 80.
	tileset = gi.nTileset;							// 0 <= x <= 6
	int inEnergy = gi.nEnergy;						// 0 <= x <= 6
	int inTerrain = gi.nTerrain;					// 0 (smooth) <= x <= 6 (rough)
	int inWater = gi.nWater;						// 0 <= x <= 6
	
	if (!(inEnergy == 0)) {
		inEnergy = inEnergy*14 + plusminus(14);
	}
	
	if (!(inTerrain == 0)) {
		inTerrain = inTerrain*14 + plusminus(14);
	}
	
	if (!(inWater == 0)) {
		inWater = inWater*14 + plusminus(14);
	}
	
	float energypercent, terrainpercent, waterpercent;
	
	energypercent = ((float) inEnergy)/100;
	terrainpercent = ((float) inTerrain)/100;
	waterpercent = ((float) inWater)/100;
	
	//****************************************Let the generation BEGIN!
	
	fillboards (0);
	
	int i, j, k, l, x, y, z;										//used everywhere
	int numsplotches, length;										//used in making the basic landmass shapes
	int numwaterplaces = 0, numwatersplotches, multiplier;			//used in making water
	short int goodwater[1600][2];									//used in making water
	int maxnumclose, maxnumfar, maxnumrand, smallmed, goodplaceClose[300][2], goodplaceFar[300][2], numplaceClose, numplaceFar, placeFar[3], placeClose[2], counter, counterpools; //used in making energy
	
	//****************************************Make the basic landmass shapes.
	
	numsplotches =  (int)(size/4) + (int) ((terrainpercent-0.5)*size/2);
	
	for (i=0; i < numsplotches; i++) {
		length = (int) (((1-waterpercent)/6*size*size)/numsplotches);
		length = length + plusminus((int) (length/2));
		randomsplotch (length, 1 + plusminus(1), 2, rand()%size, rand()%size);
	}
	
	for (i=0; i < (int) numsplotches/4; i++) {
		length = (int) (((1-waterpercent)/3*size*size)/numsplotches);
		length = length + plusminus((int) (length/4));
		randomsplotch (length, 1 + plusminus(1), 1, rand()%size, rand()%size);
	}
	
	for (i=0; i < (int) numsplotches/2; i++) {
		length = (int) (size  + plusminus((int) (size/2)));
		randomsplotch (length, 1 + plusminus(1), 0, rand()%size, rand()%size);
	}
	
	for (i=0; i < (int) waterpercent*size; i++) {
		length = (int) (size  + plusminus((int) (size/2)));
		randomsplotch (length, 1 + plusminus(1), 0, rand()%size, rand()%size);
	}
	
	
	//****************************************Fatten up the landmasses
	
	fattenall(-3, 2, NEVER_USED_NUM, TEMP_REPLACEMENT_NUMA); //make the highhills bigger
	replacenum (TEMP_REPLACEMENT_NUMA, 2);
	
	fattenall(-((int) (waterpercent * 3)), 0, NEVER_USED_NUM, TEMP_REPLACEMENT_NUMA); //make the lowlands bigger, depending on wateriness
	replacenum (TEMP_REPLACEMENT_NUMA, 2);


	fattenall(2, 2, NEVER_USED_NUM, 1); //surround the high hills with at least one med
	
	fattenall(-3, 1, 2, TEMP_REPLACEMENT_NUMA); //fatten the medium hills
	replacenum (TEMP_REPLACEMENT_NUMA, 1);
	
	//****************************************Smooth out/rough up the landmasses
	
	randomflip ((int) (terrainpercent*terrainpercent*size*size/4 + size/4), inWater);
	
	//****************************************Make the start locations
	
	findstartloc();
	
	//****************************************Put down some water
	
	for (i=0; i < size; i += 2) {
		for (j=0; j < size; j += 2) {
			if (goodforwater(i, j)) {
				goodwater [numwaterplaces][0] = i;
				goodwater [numwaterplaces][1] = j;
				numwaterplaces ++;
			}
		}
	}
	
	numwatersplotches = (int) (size * waterpercent *  waterpercent * 5);
	
	
	
	if (numwaterplaces <= numwatersplotches) {
		numwatersplotches = numwaterplaces;
	}
	if (numwatersplotches >= 1) {
		multiplier = (int) ( (float) numwaterplaces/ (float) numwatersplotches);
		
		for (i=0; i<numwatersplotches; i++) {
			z = rand()%multiplier + i * multiplier;
			x = goodwater[z][0];
			y = goodwater[z][1];
			randomwater(size + plusminus (size/2), rand()%3, x, y);
		}
	}
	
	//****************************************Put down the energy
	if (inEnergy >= 90) {
		maxnumclose = 2;
		maxnumfar = 3;
		maxnumrand = (int) (size*size/150);
		smallmed = 2;
	}
	else if (inEnergy >= 75) {
		maxnumclose = 2;
		maxnumfar = 1;
		maxnumrand = (int) (size*size/250)+4;
		smallmed = 3;
	}
	else if (inEnergy >= 60) {
		maxnumclose = 1;
		maxnumfar = 2;
		maxnumrand = (int) (size*size/250);
		smallmed = 4;
	}
	else if (inEnergy >= 45) {
		maxnumclose = 1;
		maxnumfar = 1;
		maxnumrand = (int) (size*size/250);
		smallmed = 6;
	}
	else if (inEnergy >= 30) {
		maxnumclose = 1;
		maxnumfar = 0;
		maxnumrand = (int) (size*size/350)+2;
		smallmed = 8;
	}
	else if (inEnergy >= 15) {
		maxnumclose = 0;
		maxnumfar = 1;
		maxnumrand = (int) (size*size/450);
		smallmed = 10;
	}
	else if (inEnergy >= 0) {
		maxnumclose = 0;
		maxnumfar = 0;
		maxnumrand = 0;
		smallmed = 1;
	}
	
	
	for (i=0; i<4; i++) {
		numplaceClose = 0;
		numplaceFar = 0;
		for (j=0; j<size; j++) {
			for (k=0; k<size; k++) {
				if ((goodforenergy(j, k, 0)) && (distance(startloc[i][0], startloc[i][1], j, k) >= (int) (size/10)) && (distance(startloc[i][0], startloc[i][1], j, k) <= (int) (size/8))) {
					goodplaceClose[numplaceClose][0] = j;
					goodplaceClose[numplaceClose][1] = k;
					numplaceClose++;
				}
				else if ((goodforenergy(j, k, 0)) && (distance(startloc[i][0], startloc[i][1], j, k) >= (int) (size/7)) && (distance(startloc[i][0], startloc[i][1], j, k) <= (int) (size/5))) {
					goodplaceFar[numplaceFar][0] = j;
					goodplaceFar[numplaceFar][1] = k;
					numplaceFar++;
				}
			}
		}
		if (numplaceClose >= 1) {
			placeClose[0] = rand()%(int)(numplaceClose/2) + 1;
		}
		if (numplaceClose >= 2) {
			placeClose[1] = rand()%(int)(numplaceClose/2) + (int)(numplaceClose/2) - 1;
		}
		
		if (numplaceClose >= maxnumclose) {
			for (l = 0; l < maxnumclose; l++) {
				special[ goodplaceClose[placeClose[l]][0] ] [ goodplaceClose[placeClose[l]][1] ] = 100;
			}
		}
		else {
			for (l = 0; l < numplaceClose; l++) {
				special[ goodplaceClose[placeClose[l]][0] ] [ goodplaceClose[placeClose[l]][1] ] = 100;
			}
		}
		
		if (numplaceFar >= 1) {
		placeFar[0] = rand()%(int)(numplaceFar/3) + 1;
		}
		if (numplaceFar >= 2) {
		placeFar[1] = rand()%(int)(numplaceFar/3) + (int)(numplaceClose/3);
		}
		if (numplaceFar >= 3) {
		placeFar[2] = rand()%(int)(numplaceFar/3) + (int)(2 * numplaceClose/3) - 1;
		}
		
		if (numplaceFar >= maxnumfar) {
		for (l = 0; l < maxnumfar; l++) {
			special[goodplaceFar[placeFar[l]][0]][goodplaceFar[placeFar[l]][1]] = 100;	
		}
		}
		else {
		for (l = 0; l < numplaceFar; l++) {
			special[goodplaceFar[placeFar[l]][0]][goodplaceFar[placeFar[l]][1]] = 100;	
		}
		}
	}
	
	
	
	counter = 0;
	counterpools = 4 * (maxnumfar + maxnumclose);
	for (k=0; k<maxnumrand && counterpools < 50; k++) {
		do {
			i = rand()%size;
			j = rand()%size;
			counter ++;
		}
		while (!( (distance ( i, j, startloc[0][0], startloc[0][1] ) >=10) && (distance ( i, j, startloc[1][0], startloc[1][1] ) >=10) && (distance ( i, j, startloc[2][0], startloc[2][1] ) >=10) && (distance ( i, j, startloc[3][0], startloc[3][1] ) >=10) && (goodforenergy(i, j, 1) ) && (counter < 5000)));
		if (rand()%smallmed == 0 ) {
			special[i][j] = 200;
			counterpools++;
		}
		else {
			special[i][j] = 100;
			counterpools++;
		}	
	}
	
	
	//****************************************Do that saving thing that you do, BABY!
	PopulateMIF(mif);
}

int distance (int x1, int y1, int x2, int y2) {
	int dx, dy, disp;
	dx = min ((abs (x1 - x2)), (abs (x1+size - x2)), (abs (x2+size - x1)));
	dy = min ((abs (y1 - y2)), (abs (y1+size - y2)), (abs (y2+size - y1)));
	disp = (int) sqrt((double) (dx * dx + dy * dy ));
	return disp;
}

int min (int a, int b, int c) {
	if ( (a <= b) && (a <= c) ) {
		return a;
	}
	if ( (b < a) && (b <= c) ) {
		return b;
	}
	if ( (c < a) && (c < b) ) {
		return c;
	}
	else {
		return a;
	}
}

int goodforenergy (int x, int y, int poolsize) {
	switch (poolsize) {
	case 0:
		if ((board[x][y] == board[findcoord(x,+1)][y]) && (board[findcoord(x,+1)][y] == board[x][findcoord(y,+1)]) && (board[x][findcoord(y,+1)] == board[findcoord(x,+1)][findcoord(y,+1)])) { 
			//check main map
			if (special[x][y] == 0) { //specials are clear
				return 1;
			}
			else {
				return 0;
			}
		}
		else {
			return 0;
		}
	case 1:
		if ((board[x][y] == board[findcoord(x,+1)][y]) && (board[findcoord(x,+1)][y] == board[x][findcoord(y,+1)]) && (board[x][findcoord(y,+1)] == board[findcoord(x,+1)][findcoord(y,+1)])) { //check main map
			if ((special[x][y] == 0) && (special[findcoord(x,+1)][y] == 0) && (special[x][findcoord(y,+1)] == 0) && (special[findcoord(x,+1)][findcoord(y,+1)] == 0)){ //specials are clear
				return 1;
			}
			else {
				return 0;
			}
		}
		else {
			return 0;
		}
	default:
		return 0;
	}
}

int plusminus (int max) {
	int result = rand()%(max +1);
	if (rand()%2 == 0) {
		result *= (-1);
	}
	return result;
}

int fillboards (int num) {
	int i, j;
	for (i=0; i < size; i++) {
		for (j=0; j < size; j++) {
			board[i][j] = num;
			special[i][j] = num;
		}
	}
	return 0;
}

int randomplace (int numberofplaces, int placer) {
	int i, randx, randy;
	for (i=0; i< numberofplaces; i++) {
		randx = (rand()%size);
		randy = (rand()%size);
		board[randx][randy] = placer;
	}
	return 0;
}

int randomsplotch(int length, int stringiness, int placer, int x, int y) {
	int currx, curry, direction = 10, prevdirection, movex, movey, i=0;
	currx = x;
	curry = y;
	while (i<= length) {
		board[currx][curry] = placer;
		prevdirection = direction;
		direction = (rand()%4);
		if ((((direction + 2)==prevdirection)||((direction - 2) == prevdirection))&&(stringiness==2)) {
			direction = prevdirection;
		}
		if (!((((direction + 2)==prevdirection)||((direction - 2) == prevdirection))&&(stringiness==1))) {
			switch (direction) {
			case 0:
				movex=0;
				movey=1;
				break;
			case 1:
				movex=1;
				movey=0;
				break;
			case 2:
				movex=0;
				movey=-1;
				break;
			case 3:
				movex=-1;
				movey=0;
				break;
			}
			currx = findcoord(currx, movex);
			curry = findcoord(curry, movey);
			i++;
		}
	}
	return 0;
}

int findcoord (int value, int move) {
	move = move%size;
	int final = value+move;
	if (final < 0) {
		final = size + final;
	}
	if (final >= size) {
		final = final%size ;
	}
	return final;
}

int replacenum (int replacee, int replacer) {
	int i, j;
	for (j=0; j<size; j++) {
		for (i=0; i<size; i++) {
			if (board [i][j] == replacee) {
				board [i][j] = replacer;
			}
		}
	}
	return 0;
}

int fattenall (int howfat, int middle, int ignorer, int replacer) {
	int i,j, temp;
	for (j=0; j<size; j++){
		for (i=0; i<size; i++){
			if (board[i][j] == middle) {
				if (howfat <= 0) {
					temp = (int)( abs(howfat)+plusminus(2));
					if (temp <= 1) {
						temp = 2;
					}
					else if (temp >= 6) {
						temp = 5;
					}
					fattenone (i, j, temp, middle, ignorer, replacer);
				}
				else {
					fattenone (i, j, howfat, middle, ignorer, replacer);
				}
			}
		}
	}
	return 0;
}
int fattenone (int x, int y, int howfat, int middle, int ignorer, int 
			   replacer) {
	if (howfat==-100) {
		board[x][y] = replacer;
		board[findcoord(x,+1)][y] = replacer;
		board[x][findcoord(y,+1)] = replacer;
		board[findcoord(x,+1)][findcoord(y,+1)] = replacer;
	}
	if (howfat>=1) {
		board[x][findcoord(y,-1)] = ((board[x][findcoord(y,-1)]==middle)||(board[x][findcoord(y,-1)]==ignorer))?board[x][findcoord(y,-1)]:replacer;
		board[x][findcoord(y,+1)] = ((board[x][findcoord(y,+1)]==middle)||(board[x][findcoord(y,+1)]==ignorer))?board[x][findcoord(y,+1)]:replacer;
		board[findcoord(x,-1)][y] = ((board[findcoord(x,-1)][y]==middle)||(board[findcoord(x,-1)][y]==ignorer))?board[findcoord(x,-1)][y]:replacer;
		board[findcoord(x,+1)][y] = ((board[findcoord(x,+1)][y]==middle)||(board[findcoord(x,+1)][y]==ignorer))?board[findcoord(x,+1)][y]:replacer;
	}
	if (howfat>=2) {
		board[findcoord(x,-1)][findcoord(y,-1)] = ((board[findcoord(x,-1)][findcoord(y,-1)]==middle)||(board[findcoord(x,-1)][findcoord(y,-1)]==ignorer))?board[findcoord(x,-1)][findcoord(y,-1)]:replacer;
		board[findcoord(x,-1)][findcoord(y,+1)] = ((board[findcoord(x,-1)][findcoord(y,+1)]==middle)||(board[findcoord(x,-1)][findcoord(y,+1)]==ignorer))?board[findcoord(x,-1)][findcoord(y,+1)]:replacer;
		board[findcoord(x,+1)][findcoord(y,-1)] = ((board[findcoord(x,+1)][findcoord(y,-1)]==middle)||(board[findcoord(x,+1)][findcoord(y,-1)]==ignorer))?board[findcoord(x,+1)][findcoord(y,-1)]:replacer;
		board[findcoord(x,+1)][findcoord(y,+1)] = ((board[findcoord(x,+1)][findcoord(y,+1)]==middle)||(board[findcoord(x,+1)][findcoord(y,+1)]==ignorer))?board[findcoord(x,+1)][findcoord(y,+1)]:replacer;
	}
	if (howfat>=3) {
		board[x][findcoord(y,-2)] = ((board[x][findcoord(y,-2)]==middle)||(board[x][findcoord(y,-2)]==ignorer))?board[x][findcoord(y,-2)]:replacer;
		board[x][findcoord(y,+2)] = ((board[x][findcoord(y,+2)]==middle)||(board[x][findcoord(y,+2)]==ignorer))?board[x][findcoord(y,+2)]:replacer;
		board[findcoord(x,-2)][y] = ((board[findcoord(x,-2)][y]==middle)||(board[findcoord(x,-2)][y]==ignorer))?board[findcoord(x,-2)][y]:replacer;
		board[findcoord(x,+2)][y] = ((board[findcoord(x,+2)][y]==middle)||(board[findcoord(x,+2)][y]==ignorer))?board[findcoord(x,+2)][y]:replacer;
	}
	if (howfat>=4) {
		board[findcoord(x,-1)][findcoord(y,-2)] = ((board[findcoord(x,-1)][findcoord(y,-2)]==middle)||(board[findcoord(x,-1)][findcoord(y,-2)]==ignorer))?board[findcoord(x,-1)][findcoord(y,-2)]:replacer;
		board[findcoord(x,-1)][findcoord(y,+2)] = ((board[findcoord(x,-1)][findcoord(y,+2)]==middle)||(board[findcoord(x,-1)][findcoord(y,+2)]==ignorer))?board[findcoord(x,-1)][findcoord(y,+2)]:replacer;
		board[findcoord(x,+1)][findcoord(y,-2)] = ((board[findcoord(x,+1)][findcoord(y,-2)]==middle)||(board[findcoord(x,+1)][findcoord(y,-2)]==ignorer))?board[findcoord(x,+1)][findcoord(y,-2)]:replacer;
		board[findcoord(x,+1)][findcoord(y,+2)] = ((board[findcoord(x,+1)][findcoord(y,+2)]==middle)||(board[findcoord(x,+1)][findcoord(y,+2)]==ignorer))?board[findcoord(x,+1)][findcoord(y,+2)]:replacer;
		
		board[findcoord(x,-2)][findcoord(y,-1)] = ((board[findcoord(x,-2)][findcoord(y,-1)]==middle)||(board[findcoord(x,-2)][findcoord(y,-1)]==ignorer))?board[findcoord(x,-2)][findcoord(y,-1)]:replacer;
		board[findcoord(x,-2)][findcoord(y,+1)] = ((board[findcoord(x,-2)][findcoord(y,+1)]==middle)||(board[findcoord(x,-2)][findcoord(y,+1)]==ignorer))?board[findcoord(x,-2)][findcoord(y,+1)]:replacer;
		board[findcoord(x,+2)][findcoord(y,-1)] = ((board[findcoord(x,+2)][findcoord(y,-1)]==middle)||(board[findcoord(x,+2)][findcoord(y,-1)]==ignorer))?board[findcoord(x,+2)][findcoord(y,-1)]:replacer;
		board[findcoord(x,+2)][findcoord(y,+1)] = ((board[findcoord(x,+2)][findcoord(y,+1)]==middle)||(board[findcoord(x,+2)][findcoord(y,+1)]==ignorer))?board[findcoord(x,+2)][findcoord(y,+1)]:replacer;
	}
	if (howfat>=5) {
		board[findcoord(x,-2)][findcoord(y,-2)] = ((board[findcoord(x,-2)][findcoord(y,-2)]==middle)||(board[findcoord(x,-2)][findcoord(y,-2)]==ignorer))?board[findcoord(x,-2)][findcoord(y,-2)]:replacer;
		board[findcoord(x,-2)][findcoord(y,+2)] = ((board[findcoord(x,-2)][findcoord(y,+2)]==middle)||(board[findcoord(x,-2)][findcoord(y,+2)]==ignorer))?board[findcoord(x,-2)][findcoord(y,+2)]:replacer;
		board[findcoord(x,+2)][findcoord(y,-2)] = ((board[findcoord(x,+2)][findcoord(y,-2)]==middle)||(board[findcoord(x,+2)][findcoord(y,-2)]==ignorer))?board[findcoord(x,+2)][findcoord(y,-2)]:replacer;
		board[findcoord(x,+2)][findcoord(y,+2)] = ((board[findcoord(x,+2)][findcoord(y,+2)]==middle)||(board[findcoord(x,+2)][findcoord(y,+2)]==ignorer))?board[findcoord(x,+2)][findcoord(y,+2)]:replacer;
		
		board[x][findcoord(y,-3)] = ((board[x][findcoord(y,-3)]==middle)||(board[x][findcoord(y,-3)]==ignorer))?board[x][findcoord(y,-3)]:replacer;
		board[x][findcoord(y,+3)] = ((board[x][findcoord(y,+3)]==middle)||(board[x][findcoord(y,+3)]==ignorer))?board[x][findcoord(y,+3)]:replacer;
		board[findcoord(x,-3)][y] = ((board[findcoord(x,-3)][y]==middle)||(board[findcoord(x,-3)][y]==ignorer))?board[findcoord(x,-3)][y]:replacer;
		board[findcoord(x,+3)][y] = ((board[findcoord(x,+3)][y]==middle)||(board[findcoord(x,+3)][y]==ignorer))?board[findcoord(x,+3)][y]:replacer;
	}
	return 0;
}

int findstartloc (void) {
	int temp, i, j, shiftx, shifty, secondshift;
	int start[4][2];
	shiftx = rand()%size;
	shifty = rand()%size;
	start[0][0] = findcoord((int) size/4, (plusminus(3) + shiftx));
	start[0][1] = findcoord((int) size/4, (plusminus(3) + shifty));
	start[1][0] = findcoord((int) 3 * size/4, (plusminus(3) + shiftx));
	start[1][1] = findcoord((int) size/4, (plusminus(3) + shifty));
	start[2][0] = findcoord((int) size/4, (plusminus(3) + shiftx));
	start[2][1] = findcoord((int) 3 * size/4, (plusminus(3) + shifty));
	start[3][0] = findcoord((int) 3 * size/4, (plusminus(3) + shiftx));
	start[3][1] = findcoord((int) 3 * size/4, (plusminus(3) + shifty));
	
	temp = rand()%2;
	secondshift = rand()%size;
	
	if (temp == 0) {
		start[0][0] = findcoord(start[0][0], secondshift);
		start[1][0] = findcoord(start[1][0], secondshift);
	}
	
	else {
		start[1][1] = findcoord(start[1][1], secondshift);
		start[3][1] = findcoord(start[3][1], secondshift);
	}
	
	temp = whatheightstartloc(start[0][0], start[0][1]);
	fattenone (start[0][0], start[0][1], -100, temp, NEVER_USED_NUM, temp);
	
	temp = whatheightstartloc(start[1][0], start[1][1]);
	fattenone (start[1][0], start[1][1], -100, temp, NEVER_USED_NUM, temp);
	
	temp = whatheightstartloc(start[2][0], start[2][1]);
	fattenone (start[2][0], start[2][1], -100, temp, NEVER_USED_NUM, temp);
	
	temp = whatheightstartloc(start[3][0], start[3][1]);
	fattenone (start[3][0], start[3][1], -100, temp, NEVER_USED_NUM, temp);
	
	for (j=0; j < 4; j++) {
		for (i=0; i < 2; i++) {
			startloc[j][i] = start[j][i];
		}
	}
	
	temp = rand()%4;
	j = 4;
	for (i=0; i<4; i++) {
		if (temp == i) {
		}
		else {
			startloc[j][0] = start[i][0];
			startloc[j][1] = start[i][1];
			j++;
		}
	}
	
	startloc[7][0] = startloc[4][0];
	startloc[7][1] = startloc[4][1];
	startloc[8][0] = startloc[6][0];
	startloc[8][1] = startloc[6][1];
	
	for (j=9; j < 13; j++) {
		for (i=0; i < 2; i++) {
			startloc[j][i] = start[j-9][i];
		}
	}
	
	for (j=13; j < 17; j++) {
		for (i=0; i < 2; i++) {
			startloc[j][i] = start[j-13][i];
		}
	}
	
	for (j=17; j < 20; j++) {
		for (i=0; i < 2; i++) {
			startloc[j][i] = startloc[j-13][i];
		}
	}
	
	//place on special map
	for (i=0; i<4; i++) {
		special[startloc[i][0]][startloc[i][1]] += 1;
	}
	for (i=4; i<7; i++) {
		special[startloc[i][0]][startloc[i][1]] += 2;
	}
	for (i=7; i<9; i++) {
		special[startloc[i][0]][startloc[i][1]] += 4;
	}
	for (i=9; i<13; i++) {
		special[startloc[i][0]][startloc[i][1]] += 8;
	}
	for (i=13; i<17; i++) {
		special[startloc[i][0]][startloc[i][1]] += 16;
	}
	for (i=17; i<20; i++) {
		special[startloc[i][0]][startloc[i][1]] += 32;
	}
	return 0;
}

int whatheightstartloc (int x, int y) {
	int heightfield[3] = {0, 0, 0};
	
	heightfield[board[findcoord(x,+2)][findcoord(y,-1)]]++;
	heightfield[board[findcoord(x,+2)][y]]++;
	heightfield[board[findcoord(x,+2)][findcoord(y,+1)]]++;
	heightfield[board[findcoord(x,+2)][findcoord(y,+2)]]++;
	
	heightfield[board[findcoord(x,-1)][findcoord(y,-1)]]++;
	heightfield[board[findcoord(x,-1)][y]]++;
	heightfield[board[findcoord(x,-1)][findcoord(y,+1)]]++;
	heightfield[board[findcoord(x,-1)][findcoord(y,+2)]]++;
	
	heightfield[board[x][findcoord(y,-1)]]++;
	heightfield[board[x][y]]++;
	heightfield[board[x][findcoord(y,+1)]]++;
	heightfield[board[x][findcoord(y,+2)]]++;
	
	heightfield[board[findcoord(x,+1)][findcoord(y,-1)]]++;
	heightfield[board[findcoord(x,+1)][y]]++;
	heightfield[board[findcoord(x,+1)][findcoord(y,+1)]]++;
	heightfield[board[findcoord(x,+1)][findcoord(y,+2)]]++;
	
	if (heightfield[0] == 0) {
		if (heightfield[1] >= heightfield[2]) {
			return 1;
		}
		else {
			return 2;
		}
	}
	else if (heightfield[1] == 0) {
		if (heightfield[0] >= heightfield[2]) {
			return 0;
		}
		else {
			return 2;
		}
	}
	else if (heightfield[2] == 0) {
		if (heightfield[1] >= heightfield[0]) {
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		return 1;
	}
}

int goodforwater (int x,  int y) {
	if ((board[x][y] == 0) && (board[findcoord(x,+1)][y] == 0) && (board[x][findcoord(y,+1)] == 0) && (board[findcoord(x,+1)][findcoord(y,+1)] == 0)) { //check main map
		if ((special[x][y] <= 0) && (special[x][findcoord(y, 1)] <= 0) && (special[findcoord(x, 1)][findcoord(y, 1)] <= 0) && (special[findcoord(x, 1)][y] <= 0) && (special[findcoord(x, 1)][findcoord(y, -1)] <= 0) && (special[x][findcoord(y, -1)] <= 0) && (special[findcoord(x, -1)][findcoord(y, -1)] <= 0) && (special[findcoord(x, -1)][y] <= 0) && (special[findcoord(x, -1)][findcoord(y, 1)] <= 0)) { //specials are clear
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		return 0;
	}
}

int randomwater(int length, int stringiness, int x, int y) {
	int currx, curry, direction = 10, prevdirection, i=0;
	currx = x;
	curry = y;
	while (i<= length) {
		special[currx][curry] = -1;
		prevdirection = direction;
		direction = (rand()%4);
		if ((((direction + 2)==prevdirection)||((direction - 2) == prevdirection))&&(stringiness==2)) {
			direction = prevdirection;
		}
		if (!((((direction + 2)==prevdirection)||((direction - 2) == prevdirection))&&(stringiness==1))) {
			switch (direction) {
				
			case 0: //north
				if (goodforwater(currx, findcoord(curry, 1))) {
					curry = findcoord(curry, 1);
				}
				break;
				
			case 1: //east
				if (goodforwater(findcoord(currx, 1), curry)) {
					currx = findcoord(currx, 1);
				}
				break;
				
			case 2: //south
				if (goodforwater(currx, findcoord(curry, -1))) {
					curry = findcoord(curry, -1);
				}
				break;
				
			case 3: //west
				if (goodforwater(findcoord(currx, -1), curry)) {
					currx = findcoord(currx, -1);
				}
				break;
			}
			i++;
		}
	}
	return 0;
}

int tileaverage (int x, int y, int threshold) {
	int heightfield[3] = {0, 0, 0};
	
	heightfield[board[findcoord(x,-1)][findcoord(y,-1)]]++;
	heightfield[board[findcoord(x,-1)][y]]++;
	heightfield[board[findcoord(x,-1)][findcoord(y,+1)]]++;
	
	heightfield[board[x][findcoord(y,-1)]]++;
	heightfield[board[x][y]]++;
	heightfield[board[x][findcoord(y,+1)]]++;
	
	heightfield[board[findcoord(x,+1)][findcoord(y,-1)]]++;
	heightfield[board[findcoord(x,+1)][y]]++;
	heightfield[board[findcoord(x,+1)][findcoord(y,+1)]]++;
	
	if ((heightfield[2] == 0) && (heightfield[1] < heightfield[0]) && (heightfield[0] >= threshold)) {
		board[x][y] = 0;
		return 0;
	}
	else if ((heightfield[0] == 0) && (heightfield[1] < heightfield[2]) && (heightfield[2] >= threshold)) {
		board[x][y] = 2;
		return 2;
	}
	else if (heightfield[1] >= threshold) {
		board[x][y] = 1;
		return 1;
	}
	else {
		return 0;
	}
}

int randomflip (int numberofplaces, int inWater) {
	int i, x, y, temp;
	for (i = 0; i < numberofplaces; i++) {
		x = rand()%size;
		y = rand()%size;
		if (board[x][y] == 0) {
			temp = rand()%inWater;
			if (temp <= 50) {
				board[x][y] = 1;
			}
		}
		else if (board[x][y] == 2) {
			board[x][y] = 1;
		}
		else if (board[x][y] == 1) {
			
			temp = rand()%2;
			int heightfield[3] = {0, 0, 0};
			
			heightfield[board[findcoord(x,-1)][findcoord(y,-1)]]++;
			heightfield[board[findcoord(x,-1)][y]]++;
			heightfield[board[findcoord(x,-1)][findcoord(y,+1)]]++;
			
			heightfield[board[x][findcoord(y,-1)]]++;
			heightfield[board[x][findcoord(y,+1)]]++;
			
			heightfield[board[findcoord(x,+1)][findcoord(y,-1)]]++;
			heightfield[board[findcoord(x,+1)][y]]++;
			heightfield[board[findcoord(x,+1)][findcoord(y,+1)]]++;				temp = rand()%2;
			if (heightfield[0] == 0)  {
				board[x][y] = 2;
			}
			else if (heightfield[2] == 0){
				board[x][y] = 0;
			}
		}
	}
	return 0;
}

int wateraverage (int x, int y, int threshold) {
	int total = 0;
	
	total = (special[findcoord(x,-1)][findcoord(y,-1)] == -1)?total + 1:total;
	total = (special[findcoord(x,-1)][y] == -1)?total + 1:total;
	total = (special[findcoord(x,-1)][findcoord(y,+1)] == -1)?total + 1:total;
	total = (special[x][findcoord(y,-1)] == -1)?total + 1:total;
	total = (special[x][findcoord(y,+1)] == -1)?total + 1:total;
	total = (special[findcoord(x,+1)][findcoord(y,-1)] == -1)?total + 1:total;
	total = (special[findcoord(x,+1)][y] == -1)?total + 1:total;
	total = (special[findcoord(x,+1)][findcoord(y,+1)] == -1)?total + 1:total;
	
	if ((total >= threshold) && (goodforwater (x, y))) {
		special[x][y] = -1;
	}
	return 0;
}

int saveboard (void) {
	int i, j;
	FILE * savefile;
	savefile = fopen ("mapoutput.txt", "w");
	for (j=0; j < size; j++) {
		for (i=0; i < size; i++) {
			fprintf (savefile, "%d", board[i][j]);
		}
		fprintf (savefile, "\n");
	}
	fprintf(savefile, "4 Player Starts:\n(%d,%d), (%d,%d), (%d,%d), (%d,%d)\n", startloc [0][0], startloc [0][1], startloc [1][0], startloc [1][1], startloc [2][0], startloc [2][1], startloc [3][0], startloc [3][1]);
	fprintf(savefile, "3 Player Starts:\n(%d,%d), (%d,%d), (%d,%d)\n", startloc [4][0], startloc [4][1],startloc [5][0], startloc [5][1],startloc [6][0], startloc [6][1]);
	fprintf(savefile, "2 Player Starts:\n(%d,%d), (%d,%d)\n", startloc [7][0], startloc [7][1],startloc [8][0], startloc [8][1]);
	fprintf(savefile, "2v2 Player Starts:\n(%d,%d), (%d,%d), (%d,%d), (%d,%d)\n", startloc [9][0], startloc [9][1],startloc [10][0], startloc [10][1],startloc [11][0], startloc [11][1],startloc [12][0], startloc [12][1]);
	fprintf(savefile, "1v3 Player Starts:\n(%d,%d), (%d,%d), (%d,%d), (%d,%d)\n", startloc [13][0], startloc [13][1],startloc [14][0], startloc [14][1],startloc [15][0], startloc [15][1],startloc [16][0], startloc [16][1]);
	fprintf(savefile, "1v2 Player Starts:\n(%d,%d), (%d,%d), (%d,%d)\n", startloc [17][0], startloc [17][1],startloc [18][0], startloc [18][1],startloc [19][0], startloc [19][1]);
	
	for (j=0; j < size; j++) {
		for (i=0; i < size; i++) {
			if (special[j][i] == 100) {
				fprintf (savefile, "S");
			}
			else if (special[j][i] == 0) {
				fprintf (savefile, ".");
			}
			else {
				fprintf (savefile, "#");
			}
		}
		fprintf (savefile, "\n");
	}
	fclose (savefile);
	return 0;
}

