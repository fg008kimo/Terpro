#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "freegame.h"
#include "board.h"
#include "../server.h"
#include "../Alg2.h"
#include "../data.h"

//#define RECORD_WILD    				1
//#define STICK_WILD    				1
//#define PRINT_RAND2    				1
//#define PRINT_BOARD_FREEGAME   			1
//#define GEN_SCATTER						1

/////////////////////////////////【对外接口】////////////////////////////////////////

int calScatterNum(int *icons, int targetIcon)
{
	int i, num = 0;
	for(i = 0; i < BOARD_ICON_NUM; i++){
		if(icons[i] == targetIcon)
			num++;
	}
	return num;
}

void genScatter(int *scTemp, s8 isBuyFreegame)
{
	const uint32_t (*gScatterInfoR4)[2] = (isBuyFreegame ? gScatterInfoR5 : gScatterInfoR);
	int i, j, k, column, m;
	i = j = 0;
	memset(scTemp, 0, sizeof(int)*BOARD_ICON_NUM);
	
	for(column = 0; column < BOARD_COLUMN; column++){
		i = column * BOARD_ROW;
		j = (int)GetRand(BOARD_ROW);
		if(i == 0)
			scTemp[i+j] = getRandomValue(gScatterInfoR, SCATTER_WEIGHT_LEN, SCATTER_WEIGHT_SUM);
		else if(i == 1 * BOARD_ROW)
			scTemp[i+j] = getRandomValue(gScatterInfoR, SCATTER_WEIGHT_LEN, SCATTER_WEIGHT_SUM);	
		else if(i == 2 * BOARD_ROW)
			scTemp[i+j] = getRandomValue(gScatterInfoR, SCATTER_WEIGHT_LEN, SCATTER_WEIGHT_SUM);		
		else if(i == 3 * BOARD_ROW)
			scTemp[i+j] = getRandomValue(gScatterInfoR4, SCATTER_WEIGHT_LEN, SCATTER_WEIGHT_SUM);
		else if(i == 4 * BOARD_ROW)
			scTemp[i+j] = getRandomValue(gScatterInfoR5, SCATTER_WEIGHT_LEN, SCATTER_WEIGHT_SUM);		
		#ifdef GEN_SCATTER
		printf("i:%d, scTemp[i+j]:%d, line:%d in genScatter()\n", i, scTemp[i+j], __LINE__);
		#endif
	}
	
	//#ifdef GEN_SCATTER
    //for (k = 0; k < BOARD_ICON_NUM; k++)//print gen scatter board
	//	printf("k:%d, scTemp[k]:%d, line:%d in genScatter()\n", k, scTemp[k], __LINE__);
	//#endif
}

void RecordWild(int *icons, int *iconsTemp)
{
	int i;
	for (i = 0; i < BOARD_ICON_NUM; i++){
		if(icons[i] >= E_ICON_WILD){
			iconsTemp[i] = icons[i];
			#ifdef RECORD_WILD
			printf("i:%d, iconsTemp[i]:%d, icons[i]:%d, line:%d in RecordWild()\n", i, iconsTemp[i], icons[i], __LINE__);
			#endif
		}
	}
}

void StickWild(int *icons, int *iconsTemp)
{
    int i;
    for (i = 0; i < BOARD_ICON_NUM; i++){
        if(iconsTemp[i] >= E_ICON_WILD){
			icons[i] = iconsTemp[i];
			#ifdef STICK_WILD
			printf("i:%d, icons[i]:%d, iconsTemp[i]:%d, line:%d in StickWild()\n", i, icons[i], iconsTemp[i], __LINE__);
			#endif
		}
    }
}

void wildGetMultiplier1(int *icons, const uint32_t (*gMulValue)[2], const int WEIGHT_SUM)
{
	int i, mul;
	for(i = 0; i < BOARD_ICON_NUM; i++){
		if(icons[i] == E_ICON_WILD){
			#ifdef TEST_TRANVERSAL  
			mul = 2;
			//printf("test tranversal in wildGetMultiplier()\n");
			#else
			//mul = 2;  //test case
			mul = getRandomValue(gMulValue, MUL_NUM, WEIGHT_SUM);
			#endif
			
			icons[i] += mul;//mul = 2 3 5, icons[i]= 15 16 18
			#ifdef WILD_GET_MULTIPLIER
			printf("i:%d, icons[i]:%d, mul:%d in wildGetMultiplier()\n", i, icons[i], mul, __LINE__);
			#endif
		}	
	}
}


#ifdef FG_TEST
void freeGame_gen(s8 isBuyFreegame, int (*icons)[BOARD_ICON_NUM], int (*scArray)[BOARD_ICON_NUM], s16 *freegameNum, s16 *freegameSCcal, s16 *freegameSCNum, s16 *freegameAddNum, s16 *freegameWildNum, s16 *freegameWildFinalNum)//, s16 *freegameAddRound
//void freeGame_gen(s8 isBuyFreegame, int (*icons)[BOARD_ICON_NUM], s16 *freegameNum, s16 *freegameAddRound) //single calculation
#else
void freeGame_gen(s8 isBuyFreegame, int (*icons)[BOARD_ICON_NUM], int (*scArray)[BOARD_ICON_NUM], s16 *freegameNum)
#endif
{
	int round, iconsWildTemp[BOARD_ICON_NUM];
	memset(iconsWildTemp, 0, sizeof(int)*BOARD_ICON_NUM);
	*freegameNum = FREEGAME_DEFAULT_NUM;
	#ifdef FG_SC_DIST
	memset(freegameSCcal, 0, sizeof(int)*BOARD_ICON_NUM);
	#endif
	#ifdef FG_SC_NUM
	memset(freegameSCNum, 0, sizeof(int)*(BOARD_COLUMN + 1));
	#endif
	#ifdef FG_ADD_NUM
	memset(freegameAddNum, 0, sizeof(int)*BOARD_COLUMN);
	#endif
	
	for(round = 0; round < *freegameNum; round++){
		const uint16_t *FreeGameRollerIconNum = (isBuyFreegame ? gBuyFreeGameRollerIconNum : gFreeGameRollerIconNum);
		const uint8_t (*FreeGameRollerIcon)[ROLLER_ICON_LEN] = (isBuyFreegame ? gBuyFreeGameRollerIcon : gFreeGameRollerIcon);
		const uint32_t (*FreeGameWeight)[2] = (isBuyFreegame ? gBuyFreeGameWeight : gFreeGameWeight);
		const int WEIGHT_SUM = (isBuyFreegame ? BUY_FREEGAME_MUL_WEIGHT_SUM : MUL_WEIGHT_SUM);
		int i, j, iconsBefore[BOARD_ICON_NUM], wildNum = 0, scatNum = 0;
		#ifdef FREEGAME_FLOW
		printf("free game round:%d, *freegameNum:%d, line:%d in freeGame_gen()\n", round, *freegameNum, __LINE__);
		#endif
		memset(iconsBefore, 0, sizeof(int)*BOARD_ICON_NUM);
		board_gen(FreeGameRollerIcon, FreeGameRollerIconNum, iconsBefore);
		//#ifdef PRINT_BOARD_FREEGAME
		//for (j = 0; j < BOARD_ICON_NUM; j++)
		//	printf("j:%d, iconsBefore[j]:%d, line:%d in freeGame_gen()\n", j, iconsBefore[j], __LINE__);
		//#endif
		wildGetMultiplier1(iconsBefore, FreeGameWeight, WEIGHT_SUM);
		memcpy(icons[round], iconsBefore, sizeof(iconsBefore));
		RecordWild(iconsBefore, iconsWildTemp);
		StickWild(icons[round], iconsWildTemp);
		genScatter(scArray[round], isBuyFreegame);
		#ifdef FG_SC_DIST
		for(i = 0; i < BOARD_ICON_NUM; i++){
			if(scArray[round][i] == E_ICON_SC){
				freegameSCcal[i]++;
				//printf("freegameSCcal[%d]:%d, line:%d in freeGame_gen()\n", i, freegameSCcal[i], __LINE__);
			}
		}
		#endif
		scatNum = calScatterNum(scArray[round], E_ICON_SC);
		#ifdef GEN_SCATTER
		//printf("scatNum:%d, line:%d in freeGame_gen()\n", scatNum, __LINE__);
		//for (i = 0; i < BOARD_ICON_NUM; i++)
		//	printf("i:%d, scArray[round][i]:%d, line:%d in freeGame_gen()\n", i, scArray[round][i], __LINE__);
		#endif
		#ifdef FG_SC_NUM
		freegameSCNum[scatNum]++;
		//printf("freegameSCNum[%d]:%d, line:%d in freeGame_gen()\n", scatNum, freegameSCNum[scatNum], __LINE__);
		#endif
		if((scatNum >= gFreeGameInfo[0][0]) && (*freegameNum + gFreeGameInfo[0][1] <= FREEGAME_ROUND_MAX)){//scatNum = 2 3 4 5, array index = 0 1 2 3, gFreeGameInfo[X][1] = 4 8 12 20
			*freegameNum += gFreeGameInfo[scatNum - 2][1];//freeTotalRound 沒用指標 當賦值用常數時, round會到890然後程式自動segmentation fault
			//*freegameNum += 0;  //test case
			#ifdef FREEGAME_FLOW
			printf("round:%d, *freegameNum:%d, line:%d in freeGame_gen()\n", round, *freegameNum, __LINE__);
			#endif
			#ifdef FG_ADD_ROUND
			freegameAddRound[round]++;  
			printf("round:%d, freegameAddRound[round]:%d, line:%d in freeGame_gen()\n", round, freegameAddRound[round], __LINE__);
			#endif
		#ifdef FG_ADD_NUM  
			freegameAddNum[scatNum - 1]++;//freegameAddNum[X] = 1 2 3 4
			//printf("freegameAddNum[%d]:%d, line:%d in freeGame_gen()\n", scatNum - 1, freegameAddNum[scatNum - 1], __LINE__);
			//#endif
		}else{
			freegameAddNum[0]++;
			//#ifdef FG_ADD_NUM
			//printf("freegameAddNum[0]:%d, line:%d in freeGame_gen()\n", freegameAddNum[0], __LINE__);
		#endif
		}
		#if defined(FG_WILD_NUM) || defined(FG_WILD_FINAL_NUM)
		wildNum = board_calWildIconNum(icons[round], E_ICON_WILD);
		#endif
		#ifdef FG_WILD_NUM
		freegameWildNum[round] = wildNum;
		//printf("freegameWildNum[%d]:%d, wildNum:%d, line:%d in freeGame_gen()\n", round, freegameWildNum[round], wildNum, __LINE__);
		#endif
		#ifdef PRINT_BOARD_FREEGAME
		for (i = 0; i < BOARD_ICON_NUM; i++)
			printf("i:%d, icons[round][i]:%d, line:%d in freeGame_gen()\n", i, icons[round][i], __LINE__);
		#endif
		#ifdef FG_WILD_FINAL_NUM
		if(round == *freegameNum - 1){
			freegameWildFinalNum[round] = wildNum;
			//printf("round:%d, *freegameNum:%d, freegameWildFinalNum[round]:%d, wildNum:%d, line:%d in freeGame_gen()\n", round, *freegameNum, freegameWildFinalNum[round], wildNum, __LINE__);
		}
		#endif
		if(*freegameNum > FREEGAME_ROUND_MAX){
			*freegameNum = FREEGAME_ROUND_MAX;
			#ifdef FG_NUM
			//printf("*freegameNum:%d, FREEGAME_ROUND_MAX:%d, line:%d in freeGame_gen()\n", *freegameNum, FREEGAME_ROUND_MAX, __LINE__);
			#endif
		}
		if(round > FREEGAME_ROUND_MAX - 1){//round = 0~99
			#ifdef FG_NUM
			//printf("round:%d, *freegameNum:%d, line:%d in freeGame_gen()\n", round, *freegameNum, __LINE__);
			#endif
			break;
		}
	}
}

