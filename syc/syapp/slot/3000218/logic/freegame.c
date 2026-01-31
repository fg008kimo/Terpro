#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "freegame.h"
#include "board.h"
#include "../server.h"
#include "../Alg2.h"

/////////////////////////////////【对外接口】////////////////////////////////////////

void freeGame_gen(s8 isBuyFreegame, int (*freegameIcons)[MAX_CASCADE_NUM][BOARD_ICON_NUM], int (*freegameMultis)[MAX_CASCADE_NUM],
    int (*freegameRewardInfos)[MAX_CASCADE_NUM][MAX_REWARD_ICON_TYPE][3], int (*freegameMulIconMultis)[BOARD_ICON_NUM], 
    s16 *freegameNum, s16 *freegameCascadeNums, s16 (*freegameRewardIconTypes)[MAX_CASCADE_NUM], s16 *freegameMulIconNums)
{
	const uint8_t (*mainRollerIcon)[ROLLER_ICON_LEN];
	const uint16_t *mainRollerIconNum;
	const uint8_t (*secRollerIcon)[ROLLER_ICON_LEN];
	const uint16_t *secRollerIconNum;
	const uint32_t *mulIconProb;
	s16 round;

	if(isBuyFreegame)
	{
		mainRollerIcon = gBuyFreeGameMainRollerIcon;
		mainRollerIconNum = gBuyFreeGameMainRollerIconNum;
		secRollerIcon = gBuyFreeGameSecRollerIcon;
		secRollerIconNum = gBuyFreeGameSecRollerIconNum;
	}
	else
	{
		mainRollerIcon = gFreeGameMainRollerIcon;
		mainRollerIconNum = gFreeGameMainRollerIconNum;
		secRollerIcon = gFreeGameSecRollerIcon;
		secRollerIconNum = gFreeGameSecRollerIconNum;
	}
	*freegameNum = gFreeGameInfo[0][1];
	round = 0;

	while(round < *freegameNum)
	{
		if(isBuyFreegame)
			mulIconProb = gBuyFreeGameMulIconProb2[RandSelect(gBuyFreeGameMulIconProb1, 3, PROB_SUM)];
		else
			mulIconProb = gFreeGameMulIconProb2[RandSelect(gFreeGameMulIconProb1, 3, PROB_SUM)];

		board_gen(mainRollerIcon, mainRollerIconNum, secRollerIcon, secRollerIconNum, mulIconProb,
			freegameIcons[round], freegameMultis[round], freegameRewardInfos[round], freegameMulIconMultis[round], 
			&freegameCascadeNums[round], freegameRewardIconTypes[round], &freegameMulIconNums[round]);

		if(board_calIconNum(freegameIcons[round][freegameCascadeNums[round]-1], E_ICON_SCATTER) >= gFreeGameInfo[1][0]
			&& *freegameNum + gFreeGameInfo[1][1] <= FREEGAME_ROUND_MAX)
			*freegameNum += gFreeGameInfo[1][1];

		round++;
	}

	if(*freegameNum >= FREEGAME_ROUND_MAX)
		LOGE("freeGame_gen error: max round illegal");
}

