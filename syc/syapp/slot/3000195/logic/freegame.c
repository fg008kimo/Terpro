#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "freegame.h"
#include "board.h"
#include "../server.h"
#include "../Alg2.h"

void freeGame_changeWildMode(s8 isBuyFreegame, int *preIcons, int *icons, int *changeWildMode)
{
	const uint8_t (*wildMode)[5] = (isBuyFreegame ? gBuyFreeGameChangeWildMode : gFreeGameChangeWildMode);
	const uint32_t *wildModeProb = (isBuyFreegame ? gBuyFreeGameChangeWildModeProb : gFreeGameChangeWildModeProb);
	int i, index;

	index = RandSelect(wildModeProb, FREEGAME_CHANGE_WILD_MODE_NUM, FREEGAME_CHANGE_WILD_MODE_PROB_SUM);//FREEGAME_CHANGE_WILD_MODE_NUM = 15, FREEGAME_CHANGE_WILD_MODE_PROB_SUM = 10000
	printf("index:%d from freeGame_changeWildMode()\n", index);
	for(i = 0; i < BOARD_ICON_NUM; i++)//BOARD_ICON_NUM = 15
	{
		if(board_isHighIcon(preIcons[i]) && wildMode[index][preIcons[i]]){//board_isHighIcon()決定是不是PIC1~PIC4。
			icons[i] = E_ICON_WILD;
			printf("i:%d, icons[i]:%d, index:%d, preIcons[i]:%d, wildMode[index][preIcons[i]]]:%d, %d from freeGame_changeWildMode()\n", i, icons[i], index, preIcons[i], wildMode[index][preIcons[i]], __LINE__);
		}else{
			icons[i] = preIcons[i];
			printf("i:%d, icons[i]:%d, index:%d, preIcons[i]:%d, wildMode[index][preIcons[i]]]:%d, %d from freeGame_changeWildMode()\n", i, icons[i], index, preIcons[i], wildMode[index][preIcons[i]], __LINE__);
		}
	}

	for(i = 0; i < HIGH_ICON_NUM; i++){	//HIGH_ICON_NUM = 4, i=0~3對應到PIC1~PIC4
		changeWildMode[i] = wildMode[index][i+1];
		printf("i:%d, wildMode[index][i+1]:%d, from freeGame_changeWildMode()\n", i, wildMode[index][i+1]);
	}
}

/////////////////////////////////【对外接口】////////////////////////////////////////

int freeGame_calNum(int *normalIcons)
{
	int iconNum, i;

	iconNum = board_calIconNum(normalIcons, E_ICON_SCATTER);
	for(i = 0; i < FREEGAME_TYPE; i++)
	{
		if(iconNum == gFreeGameInfo[i][0]){
			printf("gFreeGameInfo[i][0]:%d, gFreeGameInfo[i][1]:%d from freeGame_calNum()\n", gFreeGameInfo[i][0], gFreeGameInfo[i][1]);
			return gFreeGameInfo[i][1];
		}
	}

	LOGE("freeGame_calNum error: %d", iconNum);
	return 0;
}

void freeGame_gen(s8 isBuyFreegame, s16 freegameNum, int (*preIcons)[BOARD_ICON_NUM], int (*icons)[BOARD_ICON_NUM], int (*changeWildMode)[HIGH_ICON_NUM])
{
	int i;

	for(i = 0; i < freegameNum; i++)
	{
		printf("free game round:%d, freegameNum:%d from freeGame_gen()\n", i, freegameNum);
		board_gen(gFreeGameRollerIcon, gFreeGameRollerIconNum, preIcons[i]);
		freeGame_changeWildMode(isBuyFreegame, preIcons[i], icons[i], changeWildMode[i]);
	}
}

