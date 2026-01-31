#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "freegame.h"
#include "board.h"
#include "../server.h"
#include "../Alg2.h"
#include "../data.h"

//#define FREEGAME_CALNUM    	1
//#define FREEGAME_FLOW    		1
//#define RECORD_WILD    			1
//#define STICK_WILD    			1

/////////////////////////////////【对外接口】////////////////////////////////////////

void RecordWild(int *icons, int *iconsWildTemp)
{
	int i;
	for (i = 0; i < BOARD_ICON_NUM; i++){
		if(icons[i] == E_ICON_WILD){
			iconsWildTemp[i] = E_ICON_WILD;
			#ifdef RECORD_WILD
			printf("i:%d, iconsWildTemp[i]:%d, line:%d in RecordWild()\n", i, iconsWildTemp[i], __LINE__);
			#endif
		}
	}
}

void StickWild(int *icons, int *iconsWildTemp)
{
    int i;
    for (i = 0; i < BOARD_ICON_NUM; i++){
        if(iconsWildTemp[i] == E_ICON_WILD){
			icons[i] = E_ICON_WILD;
			#ifdef STICK_WILD
			printf("i:%d, icons[i]:%d, line:%d in StickWild()\n", i, icons[i], __LINE__);
			#endif
		}
    }
}

int freeGame_calNum(int *normalIcons)
{
	int iconNum, i;

	iconNum = board_calIconNum(normalIcons, E_ICON_SC2);
	for(i = 0; i < FREEGAME_TYPE; i++){
		if(iconNum == gFreeGameInfo[i][0]){
			#ifdef FREEGAME_CALNUM
			printf("gFreeGameInfo[i][0]:%d, gFreeGameInfo[i][1]:%d from freeGame_calNum()\n", gFreeGameInfo[i][0], gFreeGameInfo[i][1]);
			#endif
			return gFreeGameInfo[i][1];
		}
	}

	LOGE("freeGame_calNum error: %d", iconNum);
	return 0;
}

void freeGame_gen(s8 isBuyFreegame, s16 freegameNum, int (*icons)[BOARD_ICON_NUM], int (*multiplier)[BOARD_COLUMN])
{
	int i;

	for(i = 0; i < freegameNum; i++)
	{
		int iconsBefore[BOARD_ICON_NUM];
		#ifdef FREEGAME_FLOW
		printf("free game round:%d, freegameNum:%d from freeGame_gen()\n", i, freegameNum);
		#endif
		board_gen(gFreeGameRollerIcon, gFreeGameRollerIconNum, iconsBefore);
		genMultiplier(gFreeGameWeight, FREEGAME_WEIGHT_SUM, multiplier[i]);
		memcpy(icons[i], iconsBefore, sizeof(iconsBefore));
		IconsToVSsymbol(iconsBefore, icons[i]);  
	}
}

void freeGame_genSticky(s8 isBuyFreegame, s16 freegameNumSticky, int (*icons)[BOARD_ICON_NUM])
{
	int i;
	int iconsWildTemp[BOARD_ICON_NUM];
	for (i = 0; i < BOARD_ICON_NUM; i++)
        iconsWildTemp[i] = E_ICON_NULL;
    
	for(i = 0; i < freegameNumSticky; i++)
	{
		int iconsBefore[BOARD_ICON_NUM];
		#ifdef FREEGAME_FLOW
		printf("sticky game round:%d, freegameNumSticky:%d from freeGame_genSticky()\n", i, freegameNumSticky);
		#endif
		board_gen(gStickyGameRollerIcon, gStickyGameRollerIconNum, iconsBefore);
		memcpy(icons[i], iconsBefore, sizeof(iconsBefore));
		RecordWild(iconsBefore, iconsWildTemp);
		StickWild(icons[i], iconsWildTemp);
	}
}

//void freeGame_genBonus(s8 isBuyFreegame, s16 freegameNumBonus, int (*iconsBefore)[BOARD_ICON_NUM], int (*icons)[BOARD_ICON_NUM], int (*multiplier)[BOARD_COLUMN])
//{
//	int i;
//
//	for(i = 0; i < freegameNumBonus; i++)
//	{
//		#ifdef FREEGAME_FLOW
//		printf("bonus game round:%d, freegameNumBonus:%d from freeGame_genBonus()\n", i, freegameNumBonus);
//		#endif
//		board_genBonus(gBonusGameRollerIcon, gBonusGameRollerIconNum, iconsBefore[i]);
//		memcpy(icons[i], iconsBefore[i], sizeof(iconsBefore[i]));
//		IconsToVSsymbol(iconsBefore[i], icons[i]);  
//	}
//}

