#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

#include "TestMain.h"
#include "TestRandom.h"
#include "../tools/uthash.h"
#include "../utils.h"
#include "../server.h"
#include "../algMammon.h"
#include "../Alg2.h"
#include "../data.h"

#ifdef __LINUX__
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#define MAX_SEND        1000
#define COIN_RATE       100
#define DIFF            5

#define TABLE_ID        1
#define BET_LINE_NUM    BET_LINE_TOTAL
#define MACHINE_NUMS    1        
#define TOTAL_ROUNDS    (10000000) //(56*56*57*56*60) for tranversal test 

uint32_t gWinRounds[MACHINE_NUMS];
uint32_t gFreeRounds[MACHINE_NUMS];
uint32_t gEmptyRounds[MACHINE_NUMS];
uint32_t gMaxEmptyRounds[MACHINE_NUMS];
double gTotalBet[MACHINE_NUMS];
double gTotalWin[MACHINE_NUMS];
double gFreeWin[MACHINE_NUMS];
uint32_t gMaxMultiNormal[MACHINE_NUMS];
uint32_t gMaxMultiFree[MACHINE_NUMS];
uint32_t gMinMultiFree[MACHINE_NUMS] = {0xFFFFFFFF};
double gMaxSend[MACHINE_NUMS];
double gNormalDeviation[MACHINE_NUMS];//方差
double gFreeDeviation[MACHINE_NUMS];//方差
const double gNormalExp[10] = {6.2685,6.237,6.2055,6.174,6.1425,6.111,6.0795,6.048,6.0165,5.985};
const double gFreeExp[10] = {537.3,534.6,531.9,529.2,526.5,523.8,521.1,518.4,515.7,513};

static uint32_t gMachineIndex = 0;
static uint32_t gCurDiff = DIFF;

//免费游戏
#define CHECK_FREEGAME
s32 freeGameTotalRound, freeGameFreeRound, freeGameCurSeqNonRound, freeGameMaxSeqNonRound;
double freeGameTotalMulti, freeGameFreeMulti;
s64 freeGameNumTotal, freeGameNumMax;

//dongyuan指标
#define CHECK_DONGYUAN
long dyTotalRound, dyNormalWinRound, dyFreeRound, dyFreeTotalSpins, dyTotalWinNum;
double dyTotalBet, dyTotalWin, dyNormalWin, dyFreeWin;
double dyNormalMinMulti, dyNormalMaxMulti, dyFreeMinMulti, dyFreeMaxMulti;
#define DY_PAYOUTDIST_TYPE 3
enum dy_payoutDist_types {DY_PAYOUTDIST_TYPE_TOTAL, DY_PAYOUTDIST_TYPE_NORMAL, DY_PAYOUTDIST_TYPE_FREE};
#define DY_PAYDISTVALUE_LEN 15
const int dyPayoutDistValue[DY_PAYDISTVALUE_LEN] = {1, 5, 10, 20, 30, 50, 100, 200, 300, 400, 500, 1000, 2000, 5000, 10000};
long dyPayoutDistLQRound[DY_PAYOUTDIST_TYPE][DY_PAYDISTVALUE_LEN];
long dyPayoutDistBetweenRound[DY_PAYOUTDIST_TYPE][DY_PAYDISTVALUE_LEN];
double dyPayoutDistBetweenWin[DY_PAYOUTDIST_TYPE][DY_PAYDISTVALUE_LEN];
#define DY_SYMBOL_PAYOUTDIST_TYPE 4
enum dy_symbol_payoutDist_types {DY_SYMBOL_PAYOUTDIST_TYPE_NORMAL, DY_SYMBOL_PAYOUTDIST_TYPE_FREE, DY_SYMBOL_PAYOUTDIST_TYPE_BUY_NORMAL, DY_SYMBOL_PAYOUTDIST_TYPE_BUY_FREE};
double dySymbolPayoutDistWin[DY_SYMBOL_PAYOUTDIST_TYPE][ICON_NUM+1][BOARD_COLUMN+1];
#ifdef FG_TEST
long dyFreegameNumDist[FREEGAME_ROUND_MAX], dyFreegameNumDist2[FREEGAME_ROUND_MAX / 4 - 1], dyFreegameWildNumDist[BOARD_WILD_MAX+1], dyFreegameWildFinalNumDist[BOARD_WILD_MAX+1], dyFreegameAddNumDist[FREEGAME_TYPE+1];
long long dyFreegameSCDist[BOARD_COLUMN], dyFreegameSCNum[BOARD_COLUMN+1], dyFreegameAddRd[FREEGAME_ROUND_MAX+1];
#endif
#ifdef REEL_DIST
long long dyReelDist[BOARD_COLUMN][ROLLER_ICON_LEN];
long dyReelDist1;
long dyReelDist2;
long dyReelDist3;
long dyReelDist4;
long dyReelDist5;
long dyReelDist6;
long dyReelDist7;
long dyReelDist8;
#endif
#ifdef GET_RAND
long long dyRandDist[100];
#endif
#ifdef SRAND
long long dyRandDist1[100];
#endif
#ifdef WILD_MUL_DIST
long long dywildMulDist[6];
#endif

//#define FG_PRINT							1
#ifdef FG_PRINT
	#define FG_NUM_PRINT    				1
	#define FG_WILD_NUM_PRINT    			1
	#define FG_WILD_FINAL_NUM_PRINT    	1
	#define FG_ADD_NUM_PRINT    			1
	#define FG_SC_DIST_PRINT    			1
	#define FG_SC_NUM_PRINT    			1
	//#define FG_ADD_ROUND_PRINT				1//目前是壞的
#endif
//#define REEL_DIST_PRINT    				1

void freeGame_clean()
{
#ifdef CHECK_FREEGAME
	freeGameTotalRound = 0;
	freeGameFreeRound = 0;
	freeGameCurSeqNonRound = 0;
	freeGameMaxSeqNonRound = 0;
	freeGameTotalMulti = 0;
	freeGameFreeMulti = 0;
	freeGameNumTotal = 0;
	freeGameNumMax = 0;
#endif
}

void freeGame_handle(STR_AlgKillFish* val, STR_AlgKillFishResult* ret)
{
#ifdef CHECK_FREEGAME
	freeGameTotalRound++;
	freeGameTotalMulti += ret->totalMulti;
	if(ret->prizeType == E_PRIZETYPE_FREEGAME)
	{
		freeGameFreeRound++;
		freeGameCurSeqNonRound = 0;
		freeGameFreeMulti += ret->totalMulti;
		freeGameNumTotal += ret->freegameNum;
		if(ret->freegameNum > freeGameNumMax)
			freeGameNumMax = ret->freegameNum;
	}
	else
	{
		freeGameCurSeqNonRound++;
		if(freeGameCurSeqNonRound > freeGameMaxSeqNonRound)
			freeGameMaxSeqNonRound = freeGameCurSeqNonRound;
	}
#endif
}

void freeGame_print()
{
#ifdef CHECK_FREEGAME
	LOGI("\n=====全部游戏======");
	LOGI("平均几局出一次：      %f", (double)freeGameTotalRound/freeGameFreeRound);
	LOGI("最大连续不中局数：    %d", freeGameMaxSeqNonRound);
	LOGI("平均倍率：	      %f", freeGameFreeMulti/freeGameFreeRound/BET_LINE_NUM);
	LOGI("出分占比：	      %f%%", freeGameFreeMulti*100/freeGameTotalMulti);
	LOGI("内部平均局数：	      %f", (double)freeGameNumTotal/freeGameFreeRound);
	LOGI("内部最大局数：	      %ld", freeGameNumMax);
	LOGI("====================");
#endif
}

void param_clear()
{
	freeGame_clean();
}

void param_handle(STR_AlgKillFish* val, STR_AlgKillFishResult* ret)
{
	freeGame_handle(val, ret);
}

void param_print()
{
	freeGame_print();
}

void dy_clear()
{
#ifndef CHECK_DONGYUAN
	return;
#endif
	dyTotalRound = dyNormalWinRound = dyFreeRound = dyFreeTotalSpins = dyTotalWinNum = 0;
	dyTotalBet = dyTotalWin = dyNormalWin = dyFreeWin = 0;
	dyNormalMinMulti = dyFreeMinMulti = 10000;
	dyNormalMaxMulti = dyFreeMaxMulti = 0;
	memset(dyPayoutDistLQRound, 0, sizeof(dyPayoutDistLQRound));
	memset(dyPayoutDistBetweenRound, 0, sizeof(dyPayoutDistBetweenRound));
	memset(dyPayoutDistBetweenWin, 0, sizeof(dyPayoutDistBetweenWin));
	memset(dySymbolPayoutDistWin, 0, sizeof(dySymbolPayoutDistWin));
	//memset(gWinRounds, 0, sizeof(gWinRounds));
	//memset(gFreeRounds, 0, sizeof(gFreeRounds));
	//memset(gMaxEmptyRounds, 0, sizeof(gMaxEmptyRounds));
	//memset(gTotalBet, 0, sizeof(gTotalBet));
	//memset(gTotalWin, 0, sizeof(gTotalWin));
	//memset(gMaxSend, 0, sizeof(gMaxSend));
	//memset(gMaxMultiNormal, 0, sizeof(gMaxMultiNormal));
	//memset(gMaxMultiFree, 0, sizeof(gMaxMultiFree));
#ifdef FG_TEST
	memset(dyFreegameNumDist, 0, sizeof(dyFreegameNumDist));
	memset(dyFreegameWildNumDist, 0, sizeof(dyFreegameWildNumDist));
	memset(dyFreegameWildFinalNumDist, 0, sizeof(dyFreegameWildFinalNumDist));
	memset(dyFreegameNumDist2, 0, sizeof(dyFreegameNumDist2));
	memset(dyFreegameAddNumDist, 0, sizeof(dyFreegameAddNumDist));
	memset(dyFreegameSCDist, 0, sizeof(dyFreegameSCDist));
	memset(dyFreegameSCNum, 0, sizeof(dyFreegameSCNum));
	memset(dyFreegameAddRd, 0, sizeof(dyFreegameAddRd));
#endif
#ifdef REEL_DIST
	memset(dyReelDist, 0, sizeof(dyReelDist));
	dyReelDist1 = dyReelDist2 = dyReelDist3 = dyReelDist4 = dyReelDist5 = dyReelDist6 = dyReelDist7 = dyReelDist8 = 0;
#endif
#ifdef GET_RAND
	memset(dyRandDist, 0, sizeof(dyRandDist));
#endif
#ifdef SRAND
	memset(dyRandDist1, 0, sizeof(dyRandDist1));
#endif
#ifdef WILD_MUL_DIST
	memset(dywildMulDist, 0, sizeof(dywildMulDist));
#endif
}

void dy_handle(STR_AlgKillFish* val, STR_AlgKillFishResult* ret)
{
#ifndef CHECK_DONGYUAN
	return;
#endif
	dyTotalRound++;
	if(val->buyFreegameScore > 0)
		dyTotalBet += (double)val->buyFreegameScore / val->times;
	else
		dyTotalBet += (double)val->lineNum * val->lineBet / val->times;
	dyTotalWin += (double)ret->totalMulti * val->lineBet / val->times;
	if(ret->normalMulti > 0)
	{
		dyNormalWinRound++;
		dyNormalWin += (double)ret->normalMulti * val->lineBet / val->times;
		dyNormalMinMulti = (dyNormalMinMulti > (double)ret->normalMulti / val->lineNum) ? (double)ret->normalMulti / val->lineNum : dyNormalMinMulti;
		dyNormalMaxMulti = ((double)ret->normalMulti / val->lineNum > dyNormalMaxMulti) ? (double)ret->normalMulti / val->lineNum : dyNormalMaxMulti;
	}
	if(ret->prizeType == E_PRIZETYPE_FREEGAME)
	{
		//u32 freeMulti = ret->totalMulti - ret->normalMulti;
		u32 freeMulti = 0;
		int k;
		for(k = 0; k < ret->freegameNum; k++){
			freeMulti += ret->freegameMultis[k];
			#ifdef DEBUG_PAYOUT_DIST
			printf("freeMulti:%d\n", freeMulti);
			#endif
		}			
		dyFreeRound++;
		dyFreeTotalSpins += ret->freegameNum;
		dyFreeWin += (double)freeMulti * val->lineBet / val->times;
		#ifdef DEBUG_PAYOUT_DIST
		printf("dyFreeWin:%lf\n", dyFreeWin);
		#endif
		dyFreeMinMulti = (dyFreeMinMulti > (double)freeMulti / val->lineNum) ? (double)freeMulti / val->lineNum : dyFreeMinMulti;
		dyFreeMaxMulti = ((double)freeMulti / val->lineNum > dyFreeMaxMulti) ? (double)freeMulti / val->lineNum : dyFreeMaxMulti;
		
		int i, j, w;
		#ifdef FG_NUM
		for(i = FREEGAME_DEFAULT_NUM - 1 ; i < FREEGAME_ROUND_MAX; i += 4){//cal free game num dist 
			if(i == ret->freegameNum - 1){//i is array index, not actually number(i must correspond to array index)
				dyFreegameNumDist[i]++;
				#ifdef FG_NUM_PRINT
				printf("ret->freegameNum:%d, dyFreegameNumDist[%d]:%d, dyFreegameNumDist[FREEGAME_ROUND_MAX]:%d, FREEGAME_ROUND_MAX:%d, line:%d in FG_NUM part\n", ret->freegameNum, i, dyFreegameNumDist[i], dyFreegameNumDist[FREEGAME_ROUND_MAX], FREEGAME_ROUND_MAX, __LINE__);
				#endif
				//ret->freegameNum = 0; //不能 = 0,因為下面要用,除非換變數名
				break;
			}	
		}
		#endif
		#ifdef FG_WILD_NUM
		//printf("FG_WILD_NUM start, ret->freegameNum:%d, line:%d in FG_WILD_NUM part\n", ret->freegameNum, __LINE__);
		for(w = 0; w < ret->freegameNum; w++){//cal the wild num dist in free game
			//printf("w:%d, ret->freegameNum:%d, line:%d in FG_WILD_NUM part\n", w, ret->freegameNum, __LINE__);
			for(i = 0; i <= BOARD_WILD_MAX; i++){
				//printf("i:%d, w:%d, ret->freegameNum:%d, line:%d in FG_WILD_NUM part\n", i, w, ret->freegameNum, __LINE__);
				if(i == ret->freegameWildNum[w]){
					dyFreegameWildNumDist[i]++;
					#ifdef FG_WILD_NUM_PRINT
					printf("w:%d, dyFreegameWildNumDist[%d]:%d, line:%d in FG_WILD_NUM part\n", w, i, dyFreegameWildNumDist[i], __LINE__);
					#endif
					break;
				}
			}
		}
		#endif
		#ifdef FG_WILD_FINAL_NUM
		if(j = ret->freegameNum - 1){//cal the last free game wild num dist in free game
			for(i = 0; i <= BOARD_WILD_MAX; i++){
				if(i == ret->freegameWildFinalNum[j]){
					dyFreegameWildFinalNumDist[i]++;
					#ifdef FG_WILD_FINAL_NUM_PRINT
					printf("j:%d, dyFreegameWildFinalNumDist[%d]:%d, line:%d in FG_WILD_FINAL_NUM part\n", j, i, dyFreegameWildFinalNumDist[i], __LINE__);
					#endif
					break;
				}
			}
		}
		#endif
		#ifdef FG_ADD_NUM
		for(j = 0; j <= FREEGAME_TYPE; j++){//cal free game add game num dist
			dyFreegameAddNumDist[j] += ret->freegameAddNum[j];
			#ifdef FG_ADD_NUM_PRINT
			printf(" dyFreegameAddNumDist[%d]:%d, line:%d in FG_ADD_NUM part\n", j, dyFreegameAddNumDist[j], __LINE__);
			#endif
			ret->freegameAddNum[j] = 0;
		}
		#endif
		#ifdef FG_SC_DIST
		for(j = 0; j < BOARD_COLUMN; j++){//sum up free game scatter on every reel
			dyFreegameSCDist[j] += ret->freegameSCcal[j];
			#ifdef FG_SC_DIST_PRINT
			printf(" dyFreegameSCDist[%d]:%d, line:%d in FG_SC_DIST part\n", j, dyFreegameSCDist[j], __LINE__);
			#endif
			ret->freegameSCcal[j] = 0;
		}
		#endif
		#ifdef FG_SC_NUM
		for(i = 0; i <= BOARD_COLUMN; i++){
			if(ret->freegameSCNum[i] != 0){
				dyFreegameSCNum[i] += ret->freegameSCNum[i]; 
				#ifdef FG_SC_NUM_PRINT
				printf("i:%d, ret->freegameSCNum[i]:%d, dyFreegameSCNum[i]:%d, line:%d in FG_SC_NUM part\n", i, ret->freegameSCNum[i], dyFreegameSCNum[i], __LINE__);
				#endif
				ret->freegameSCNum[i] = 0;
			}
		}
		#endif
		#ifdef FG_ADD_ROUND
		for(i = 0; i <= FREEGAME_ROUND_MAX; i++){
			if(ret->freegameAddRound[i] != 0){
				dyFreegameAddRd[i] += ret->freegameAddRound[i]; 
				#ifdef FG_ADD_ROUND_PRINT
				printf("i:%d, ret->freegameAddRound[i]:%d, dyFreegameAddRd[i]:%d, line:%d in FG_ADD_ROUND part\n", i, ret->freegameAddRound[i], dyFreegameAddRd[i], __LINE__);
				#endif
				ret->freegameAddRound[i] = 0;
			}
		}
		#endif		
	}
	#ifdef REEL_DIST
	int l, m;
	for(l = 0; l < BOARD_COLUMN; l++){//cal reel position dist 
		for(m = 0; m < gRollerIconNum[l]; m++){
			if(ret->rollerDist[l][m] != 0){
				dyReelDist[l][m] += ret->rollerDist[l][m]; 
				#ifdef REEL_DIST_PRINT
				printf("l:%d, m:%d, dyReelDist[l][m]:%d, line:%d in REEL_DIST part\n", l, m, dyReelDist[l][m], __LINE__);
				#endif
				ret->rollerDist[l][m] = 0;
			}
			if(ret->rollerDist1 != 0){
				dyReelDist1 += ret->rollerDist1; 
				//printf("dyReelDist1:%d, line:%d in REEL_DIST part\n", dyReelDist1, __LINE__);
				ret->rollerDist1 = 0;
			}
			if(ret->rollerDist2 != 0){
				dyReelDist2 += ret->rollerDist2; 
				//printf("dyReelDist2:%d, line:%d in REEL_DIST part\n", dyReelDist2, __LINE__);
				ret->rollerDist2 = 0;
			}
			if(ret->rollerDist3 != 0){
				dyReelDist3 += ret->rollerDist3; 
				//printf("dyReelDist3:%d, line:%d in REEL_DIST part\n", dyReelDist3, __LINE__);
				ret->rollerDist3 = 0;
			}
			if(ret->rollerDist4 != 0){
				dyReelDist4 += ret->rollerDist4;
				//printf("dyReelDist4:%d, line:%d in REEL_DIST part\n", dyReelDist4, __LINE__);
				ret->rollerDist4 = 0;
			}
			if(ret->rollerDist5 != 0){
				dyReelDist5 += ret->rollerDist5;
				//printf("dyReelDist5:%d, line:%d in REEL_DIST part\n", dyReelDist5, __LINE__);
				ret->rollerDist5 = 0;
			}
			if(ret->rollerDist6 != 0){
				dyReelDist6 += ret->rollerDist6;
				//printf("dyReelDist6:%d, line:%d in REEL_DIST part\n", dyReelDist6, __LINE__);
				ret->rollerDist6 = 0;
			}
			if(ret->rollerDist7 != 0){
				dyReelDist7 += ret->rollerDist7;
				//printf("dyReelDist7:%d, line:%d in REEL_DIST part\n", dyReelDist7, __LINE__);
				ret->rollerDist7 = 0;
			}
			if(ret->rollerDist8 != 0){
				dyReelDist8 += ret->rollerDist8;
				//printf("dyReelDist8:%d, line:%d in REEL_DIST part\n", dyReelDist8, __LINE__);
				ret->rollerDist8 = 0;
			}
		}
	}	
	#endif
	int i;
	for(i = 0; i < 100; i++){
		#ifdef GET_RAND
		if(ret->rand_num == i){
			dyRandDist[i]++;
			//printf("i:%d, ret->rand_num:%d, dyRandDist[i]:%d, line:%d\n", i, ret->rand_num, dyRandDist[i], __LINE__);
			ret->rand_num = 0;
		}
		#endif
		#ifdef SRAND
		if(ret->rand_num1 == i){
			dyRandDist1[i]++;
			//printf("i:%d, ret->rand_num1:%d, dyRandDist1[i]:%d, line:%d\n", i, ret->rand_num1, dyRandDist1[i], __LINE__);
			ret->rand_num1 = 0;
		}
		#endif
	}
	#ifdef WILD_MUL_DIST
	for(i = 1; i <= 5; i++){
		//printf("i:%d, ret->wildMul[i]:%d, line:%d\n", i, ret->wildMul[i], __LINE__);
		if(ret->wildMul[i] != 0){
			dywildMulDist[i] += ret->wildMul[i];
			//printf("i:%d, ret->wildMul[i]:%d, dywildMulDist[i]:%d, line:%d\n", i, ret->wildMul[i], dywildMulDist[i], __LINE__);
			ret->wildMul[i] = 0;
		}
	}
	#endif
	
	if(ret->totalMulti > 0)
		dyTotalWinNum++;
	
	int type;
	for(type = 0; type < DY_PAYOUTDIST_TYPE; type++)
	{
		int multi, index;
		double win;

		if((type == DY_PAYOUTDIST_TYPE_FREE && ret->prizeType == E_PRIZETYPE_NORMAL))
			continue;

		switch(type)
		{
			case DY_PAYOUTDIST_TYPE_TOTAL:
				multi = ret->totalMulti / val->lineNum;
				win = (double)ret->totalMulti * val->lineBet / val->times;
				break;
			case DY_PAYOUTDIST_TYPE_NORMAL:
				multi = ret->normalMulti / val->lineNum;
				win = (double)ret->normalMulti * val->lineBet / val->times;
				break;
			default://free game
				multi = (ret->totalMulti - ret->normalMulti) / val->lineNum;
				win = (double)(ret->totalMulti - ret->normalMulti) * val->lineBet / val->times;
				break;
		}

		for(index = 0; index < DY_PAYDISTVALUE_LEN; index++)
		{
			if(multi >= dyPayoutDistValue[index])
				dyPayoutDistLQRound[type][index]++;
		}
		for(index = 0; index < DY_PAYDISTVALUE_LEN; index++)
		{
			if(dyPayoutDistValue[index] > multi)
			{
				dyPayoutDistBetweenRound[type][index]++;
				dyPayoutDistBetweenWin[type][index] += win;
				break;
			}
		}
		if(index == DY_PAYDISTVALUE_LEN)
		{
			LOGE("dyPayoutDistBetweenRound error: multi=%d\n", multi);
		}
	}

	for(type = 0; type < DY_SYMBOL_PAYOUTDIST_TYPE; type++)
	{
		double win;
		int i, j, k;

		if((val->buyFreegameScore == 0 && type >= DY_SYMBOL_PAYOUTDIST_TYPE_BUY_NORMAL) ||
			(val->buyFreegameScore > 0 && type < DY_SYMBOL_PAYOUTDIST_TYPE_BUY_NORMAL))
			continue;

		if((type % 2 > 0) && ret->prizeType == E_PRIZETYPE_NORMAL)
			continue;

		if(type == 0)
		{
			for(i = 0; i < ret->normalLineNum; i++)
			{
				int num, icon = ret->normalLineInfos[i][1];
				for(num = 0; num <= BOARD_COLUMN; num++)
				{
					if(gIconMulti[icon][num] == ret->normalLineInfos[i][2])
						break;
				}
				if(ret->normalLineInfos[i][3] != 0){
					dySymbolPayoutDistWin[type][icon][num] += (double)gIconMulti[icon][num] * ret->normalLineInfos[i][3] * val->lineBet / val->times;
					#ifdef DEBUG_PAYOUT_DIST
					printf("i(normalLineNum):%d, icon:%d, num:%d, dySymbolPayoutDistWin[type][icon][num]:%lf, gIconMulti[icon][num]:%d, ret->normalLineInfos[i][3]:%d, line:%d\n", i, icon, num, dySymbolPayoutDistWin[type][icon][num], gIconMulti[icon][num], ret->normalLineInfos[i][3], __LINE__);
					#endif					
				}else if(ret->normalLineInfos[i][3] == 0){
					dySymbolPayoutDistWin[type][icon][num] += (double)gIconMulti[icon][num] * val->lineBet / val->times;
					#ifdef DEBUG_PAYOUT_DIST
					printf("i(normalLineNum):%d, icon:%d, num:%d, dySymbolPayoutDistWin[type][icon][num]:%lf, gIconMulti[icon][num]:%d, line:%d\n", i, icon, num, dySymbolPayoutDistWin[type][icon][num], gIconMulti[icon][num], __LINE__);
					#endif	
				}
			}
		}
		else
		{
			//for(k = 0; k < ret->freegameNum; k++){
			//	//for(i = 0; i < ret->freegameLineNum[k]; i++)
			//	printf("k:%d, ret->freegameLineNum[k]:%d, line:%d\n", k, ret->freegameLineNum[k], __LINE__);
			//}
			if(ret->prizeType == E_PRIZETYPE_FREEGAME){
				#ifdef DEBUG_PAYOUT_DIST
				printf("type:%d, line:%d\n", type, __LINE__);
				#endif
				for(k = 0; k < ret->freegameNum; k++)
				{
					for(j = 0; j < ret->freegameLineNum[k]; j++)
					{
						//if((ret->freegameMultis[k] != 0) && (ret->freegameLineNum[k] != 0)){
						int num, icon = ret->freegameLineInfos[k][j][1];
						for(num = 0; num <= BOARD_COLUMN; num++)
						{
							if(gIconMulti[icon][num] == ret->freegameLineInfos[k][j][2])
								break;
						}
						if(ret->freegameLineInfos[k][j][3] != 0){
							dySymbolPayoutDistWin[type][icon][num] += (double)gIconMulti[icon][num] * ret->freegameLineInfos[k][j][3] * val->lineBet / val->times;
							#ifdef DEBUG_PAYOUT_DIST
							printf("k(freegameNum):%d, j:%d, icon:%d, num:%d, dySymbolPayoutDistWin[type][icon][num]:%lf, gIconMulti[icon][num]:%d, ret->freegameLineInfos[k][j][3]:%d, ret->freegameLineNum[k]:%d, ret->freegameNum:%d, line:%d\n", k, j, icon, num, dySymbolPayoutDistWin[type][icon][num], gIconMulti[icon][num], ret->freegameLineInfos[k][j][3], ret->freegameLineNum[k], ret->freegameNum, __LINE__);
							#endif
						}else if(ret->freegameLineInfos[k][j][3] == 0){
							dySymbolPayoutDistWin[type][icon][num] += (double)gIconMulti[icon][num] * val->lineBet / val->times;
							#ifdef DEBUG_PAYOUT_DIST
							printf("k(freegameNum):%d, j:%d, icon:%d, num:%d, dySymbolPayoutDistWin[type][icon][num]:%lf, gIconMulti[icon][num]:%d, ret->freegameLineNum[k]:%d, ret->freegameNum:%d, line:%d\n", k, j, icon, num, dySymbolPayoutDistWin[type][icon][num], gIconMulti[icon][num], ret->freegameLineNum[k], ret->freegameNum, __LINE__);
							#endif
						}
						//}
					}
				}
			}
		}
	}
}

void dy_print()
{
#ifndef CHECK_DONGYUAN
	return;
#endif
	LOGI("\n=====东源参数======");
	LOGI("一般游戏RTP: %.4f%%", dyNormalWin / dyTotalBet * 100);
	LOGI("免费游戏RTP: %.4f%%", dyFreeWin / dyTotalBet * 100);
	LOGI("游戏整体RTP: %.4f%%\n", dyTotalWin / dyTotalBet * 100);

	LOGI("免费游戏平均赔付倍数: %.4f", dyFreeWin / dyFreeRound / (dyTotalBet / dyTotalRound));
	LOGI("免费游戏平均场次: %.4f", (double)dyFreeTotalSpins / dyFreeRound);
	LOGI("一般游戏贏錢率: %.4f%%", (double)dyNormalWinRound / dyTotalRound * 100);
	LOGI("免费游戏觸發率: %.4f%%", (double)dyFreeRound / dyTotalRound * 100);
	LOGI("整體游戏贏錢率: %.4f%%\n", (double)dyTotalWinNum / dyTotalRound * 100);

	LOGI("一般游戏最低倍数: %.4f", dyNormalMinMulti);
	LOGI("一般游戏最高倍数: %.4f", dyNormalMaxMulti);
	LOGI("免费游戏最低倍数: %.4f", dyFreeMinMulti);
	LOGI("免费游戏最高倍数: %.4f\n", dyFreeMaxMulti);

	int type;
	//LOGI("[开奖分布]");
	//char *titles[DY_PAYOUTDIST_TYPE] = {"游戏整体:", "一般游戏:", "免费游戏:"};
	//long totalRound[DY_PAYOUTDIST_TYPE] = {dyTotalRound, dyTotalRound, dyFreeRound};
	//for(type = 0; type < DY_PAYOUTDIST_TYPE; type++)
	//{
	//	int i;
	//	LOGI("%s\n---------------------------------------------------------------------------------\n| 赢倍 | 赢分  | HIT RATE   |累计机率| 赢倍区间  | HIT RATE   | 机率   | RTP    |", titles[type]);
	//	for(i = DY_PAYDISTVALUE_LEN - 1; i >= 0; i--)
	//	{
	//		LOGI("| %4d | %5d | %10.2f | %5.2f%% | %4d-%-4d | %10.2f | %5.2f%% | %5.2f%% |", 
	//			dyPayoutDistValue[i], dyPayoutDistValue[i] * BET_LINE_NUM, (dyPayoutDistLQRound[type][i] == 0 ? 0 : (double)totalRound[type] / dyPayoutDistLQRound[type][i]), (double)dyPayoutDistLQRound[type][i] / totalRound[type] * 100,
	//			(i == 0 ? 0 : dyPayoutDistValue[i-1]), dyPayoutDistValue[i], (dyPayoutDistBetweenRound[type][i] == 0 ? 0 : (double)totalRound[type] / dyPayoutDistBetweenRound[type][i]), (double)dyPayoutDistBetweenRound[type][i] / totalRound[type] * 100, dyPayoutDistBetweenWin[type][i] / dyTotalBet * 100);
	//	}
	//	LOGI("---------------------------------------------------------------------------------");
	//}

	LOGI("\n[图标开奖分布]");
	char *titles2[DY_SYMBOL_PAYOUTDIST_TYPE] = {"一般游戏:", "免费游戏:", "一般游戏(购买免费游戏时):", "免费游戏(购买免费游戏时):"};
	char *symbolStr[ICON_NUM+1] = {"", "PIPC1", "PIPC2", "PIPC3", "PIPC4", "PIPC5", "PIPC6", "A", "K", "Q", "J", "10"};
	for(type = 0; type < DY_SYMBOL_PAYOUTDIST_TYPE; type++){
		int icon;
		LOGI("%s\n------------------------------------------------------\n| SYMBOL|   1of   |   2of   |   3of   |   4of   |   5of   |", titles2[type]);
		for(icon = E_ICON_PIPC1; icon <= E_ICON_10; icon++)
		{
			LOGI("| %5s | %5.4f%% | %5.4f%% | %5.4f%% | %5.4f%% | %5.4f%% |", symbolStr[icon],
				dySymbolPayoutDistWin[type][icon][1] / dyTotalBet * 100,
				dySymbolPayoutDistWin[type][icon][2] / dyTotalBet * 100,
				dySymbolPayoutDistWin[type][icon][3] / dyTotalBet * 100,
				dySymbolPayoutDistWin[type][icon][4] / dyTotalBet * 100,
				dySymbolPayoutDistWin[type][icon][5] / dyTotalBet * 100);
		}
		LOGI("------------------------------------------------------");
	}
	#ifdef FG_NUM
	LOGI("\n[免費遊戲場數分布]");
	char *symbolStr1[PRINT_MAX / 4 - 1] = {"8", "12", "16", "20", "24", "28", "32", "36", "40", "44", "48", "52", "56", "60", "64", "68", "72", "76", "80", "84", "88", "92", "96", "100"};
	int num, i;
	LOGI("\n------------------------------------------------------\n| FreeGame NUM|     number|");
	for(i = FREEGAME_DEFAULT_NUM - 1; i < FREEGAME_ROUND_MAX; i += 4){//(i = 7 ~ 99)shrink the array size
		dyFreegameNumDist2[(i + 1)/ 4 - 2] = dyFreegameNumDist[i];
		#ifdef FG_NUM_PRINT
		printf("dyFreegameNumDist2[i/4-2]:%d, dyFreegameNumDist[i]:%d, i:%d\n", dyFreegameNumDist2[(i + 1) / 4 - 2], dyFreegameNumDist[i], i);
		#endif
	}
	for(num = 0; num < FREEGAME_ROUND_MAX / 4 - 1; num ++){//(num = 0 ~ 24)print free game number dist
		LOGI("| %11s | %10d|", symbolStr1[num], dyFreegameNumDist2[num]);
		#ifdef FG_NUM_PRINT
		printf("\n dyFreegameNumDist2[num]:%d, num:%d\n", dyFreegameNumDist2[num], num);
		#endif
	}
	LOGI("------------------------------------------------------");
	#endif
	#ifdef FG_WILD_NUM
	LOGI("\n[免費遊戲百搭分布]");
	char *symbolStr2[BOARD_WILD_MAX+1] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"};
	int wildNum;
	LOGI("\n------------------------------------------------------\n| Wild Dist|     number|");
	for(wildNum = 0; wildNum <= BOARD_WILD_MAX; wildNum++){//print free game wild number dist
		LOGI("| %7s | %10d|", symbolStr2[wildNum], dyFreegameWildNumDist[wildNum]);
		#ifdef FG_WILD_NUM_PRINT
		printf("\n dyFreegameWildNumDist[num]:%d, num:%d\n", dyFreegameWildNumDist[wildNum], wildNum);
		#endif
	}
	LOGI("------------------------------------------------------");	
	#endif
	#ifdef FG_WILD_FINAL_NUM
	LOGI("\n[免費遊戲最後一局百搭分布]");
	char *symbolStr3[BOARD_WILD_MAX+1] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"};
	int wildNum1;
	LOGI("\n------------------------------------------------------\n| Wild Dist|     number|");
	for(wildNum1 = 0; wildNum1 <= BOARD_WILD_MAX; wildNum1++){//print the last free game wild number dist
		LOGI("| %7s | %10d|", symbolStr3[wildNum1], dyFreegameWildFinalNumDist[wildNum1]);
		#ifdef FG_WILD_FINAL_NUM_PRINT
		printf("\n dyFreegameWildFinalNumDist[num]:%d, num:%d\n", dyFreegameWildFinalNumDist[wildNum1], wildNum1);
		#endif
	}
	LOGI("------------------------------------------------------");	
	#endif
	#ifdef FG_ADD_NUM
	LOGI("\n[免費遊戲加局分布]");
	char *symbolStr4[FREEGAME_TYPE+1] = {"0", "4", "8", "12", "20"};
	int addNum;
	LOGI("\n------------------------------------------------------\n|  Add NUM|      number|");
	for(addNum = 0; addNum <= FREEGAME_TYPE; addNum++){//print free game add number dist
		LOGI("| %7s | %10d|", symbolStr4[addNum], dyFreegameAddNumDist[addNum]);
		#ifdef FG_ADD_NUM_PRINT
		printf("\n dyFreegameAddNumDist[addNum]:%d, addNum:%d\n", dyFreegameAddNumDist[addNum], addNum);
		#endif
	}
	LOGI("------------------------------------------------------");	
	#endif
	#ifdef FG_SC_DIST
	LOGI("\n[免費遊戲各輪帶SC分布]");
	char *symbolStr5[BOARD_COLUMN] = {"1", "2", "3", "4", "5"};
	int SCNum;
	LOGI("\n------------------------------------------------------\n|  Reel   | SC  number|");
	for(SCNum = 0; SCNum < BOARD_COLUMN; SCNum++){//print free game SC on each reel dist
		LOGI("| %7s | %10d|", symbolStr5[SCNum], dyFreegameSCDist[SCNum]);
		#ifdef FG_SC_DIST_PRINT
		printf("\n dyFreegameSCDist[SCNum]:%d, SCNum:%d\n", dyFreegameSCDist[SCNum], SCNum);
		#endif
	}
	LOGI("------------------------------------------------------");	
	#endif
	#ifdef FG_SC_DIST
	LOGI("\n[免費遊戲SC數量分布]");
	char *symbolStr6[BOARD_COLUMN+1] = {"0", "1", "2", "3", "4", "5"};
	int SCDist;
	LOGI("\n------------------------------------------------------\n| SC Dist | SC  number|");
	for(SCDist = 0; SCDist <= BOARD_COLUMN; SCDist++){//print free game SC number dist
		LOGI("| %7s | %10d|", symbolStr6[SCDist], dyFreegameSCNum[SCDist]);
		#ifdef FG_SC_DIST_PRINT
		printf("\n dyFreegameSCNum[SCDist]:%d, SCDist:%d\n", dyFreegameSCNum[SCDist], SCDist);
		#endif
	}
	LOGI("------------------------------------------------------");	
	#endif
	#ifdef FG_ADD_ROUND
	LOGI("\n[免費遊戲加場在第幾局分布]");
	char *symbolStr10[FREEGAME_ROUND_MAX+1] = { \
		"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", \
			 "11", "12", "13", "14", "15", "16", "17", "18", "19", "20" \
			 "21", "22", "23", "24", "25", "26", "27", "28", "29", "30" \
			 "31", "32", "33", "34", "35", "36", "37", "38", "39", "40" \
			 "41", "42", "43", "44", "45", "46", "47", "48", "49", "50" \
			 "51", "52", "53", "54", "55", "56", "57", "58", "59", "60" \
			 "61", "62", "63", "64", "65", "66", "67", "68", "69", "70" \
			 "71", "72", "73", "74", "75", "76", "77", "78", "79", "80" \
			 "81", "82", "83", "84", "85", "86", "87", "88", "89", "90" \
			 "91", "92", "93", "94", "95", "96", "97", "98", "99", "100"};
	int addRound;
	LOGI("\n------------------------------------------------------\n| Round   |   number|");
	for(addRound = 0; addRound <= FREEGAME_ROUND_MAX; addRound++){//print free game SC number dist
		LOGI("| %7s | %10d|", symbolStr10[addRound], dyFreegameAddRd[addRound]);
		#ifdef FG_ADD_ROUND_PRINT
		printf("\n dyFreegameAddRd[addRound]:%d, addRound:%d\n", dyFreegameAddRd[addRound], addRound);
		#endif
	}
	LOGI("------------------------------------------------------");	
	#endif	
	#ifdef REEL_DIST
	LOGI("\n[輪帶位置分布]");
	char *symbolStr7[BOARD_COLUMN][61] = { \
		{"1 00", "1 01", "1 02", "1 03", "1 04", "1 05", "1 06", "1 07", "1 08", "1 09", "1 10", "1 11", "1 12", "1 13", "1 14", "1 15", "1 16", "1 17", "1 18", "1 19", "1 20", "1 21", "1 22", \
		"1 23", "1 24", "1 25", "1 26", "1 27", "1 28", "1 29", "1 30", "1 31", "1 32", "1 33", "1 34", "1 35", "1 36", "1 37", "1 38", "1 39", "1 40", "1 41", "1 42", "1 43", "1 44", "1 45", \
		"1 46", "1 47", "1 48", "1 49", "1 50", "1 51", "1 52", "1 53", "1 54", "1 55", "1 56", "1 57", "1 58", "1 59", "1 60"}, \
		{"2 00", "2 01", "2 02", "2 03", "2 04", "2 05", "2 06", "2 07", "2 08", "2 09", "2 10", "2 11", "2 12", "2 13", "2 14", "2 15", "2 16", "2 17", "2 18", "2 19", "2 20", "2 21", "2 22", \
		"2 23", "2 24", "2 25", "2 26", "2 27", "2 28", "2 29", "2 30", "2 31", "2 32", "2 33", "2 34", "2 35", "2 36", "2 37", "2 38", "2 39", "2 40", "2 41", "2 42", "2 43", "2 44", "2 45", \
		"2 46", "2 47", "2 48", "2 49", "2 50", "2 51", "2 52", "2 53", "2 54", "2 55", "2 56", "2 57", "2 58", "2 59", "2 60"}, \
		{"3 00", "3 01", "3 02", "3 03", "3 04", "3 05", "3 06", "3 07", "3 08", "3 09", "3 10", "3 11", "3 12", "3 13", "3 14", "3 15", "3 16", "3 17", "3 18", "3 19", "3 20", "3 21", "3 22", \
		"3 23", "3 24", "3 25", "3 26", "3 27", "3 28", "3 29", "3 30", "3 31", "3 32", "3 33", "3 34", "3 35", "3 36", "3 37", "3 38", "3 39", "3 40", "3 41", "3 42", "3 43", "3 44", "3 45", \
		"3 46", "3 47", "3 48", "3 49", "3 50", "3 51", "3 52", "3 53", "3 54", "3 55", "3 56", "3 57", "3 58", "3 59", "3 60"}, \
		{"4 00", "4 01", "4 02", "4 03", "4 04", "4 05", "4 06", "4 07", "4 08", "4 09", "4 10", "4 11", "4 12", "4 13", "4 14", "4 15", "4 16", "4 17", "4 18", "4 19", "4 20", "4 21", "4 22", \
		"4 23", "4 24", "4 25", "4 26", "4 27", "4 28", "4 29", "4 30", "4 31", "4 32", "4 33", "4 34", "4 35", "4 36", "4 37", "4 38", "4 39", "4 40", "4 41", "4 42", "4 43", "4 44", "4 45", \
		"4 46", "4 47", "4 48", "4 49", "4 50", "4 51", "4 52", "4 53", "4 54", "4 55", "4 56", "4 57", "4 58", "4 59", "4 60"}, \
		{"5 00", "5 01", "5 02", "5 03", "5 04", "5 05", "5 06", "5 07", "5 08", "5 09", "5 10", "5 11", "5 12", "5 13", "5 14", "5 15", "5 16", "5 17", "5 18", "5 19", "5 20", "5 21", "5 22", \
		"5 23", "5 24", "5 25", "5 26", "5 27", "5 28", "5 29", "5 30", "5 31", "5 32", "5 33", "5 34", "5 35", "5 36", "5 37", "5 38", "5 39", "5 40", "5 41", "5 42", "5 43", "5 44", "5 45", \
		"5 46", "5 47", "5 48", "5 49", "5 50", "5 51", "5 52", "5 53", "5 54", "5 55", "5 56", "5 57", "5 58", "5 59", "5 60"}};//base game only					  
	int r, j;
	LOGI("\n------------------------------------------------------\n|Reel  pos|     number|");
	for(r = 0; r < BOARD_COLUMN; r++){//print reel posiition dist
		for(j = 0; j < gRollerIconNum[r]; j++){
			LOGI("| %7s | %10d|", symbolStr7[r][j], dyReelDist[r][j]);
			#ifdef REEL_DIST_PRINT
			printf("\n dyReelDist[r][j]:%d, r:%d, j:%d\n", dyReelDist[r], r, j);
			#endif
		}
	}
	LOGI("------------------------------------------------------");	
	//dyReelDist1
	LOGI("\n[輪帶:r1 r2 r3 隨機數 = 0, r4隨機數 != 0分布]");
	LOGI("\n------------------------------------------------------\n|     number|");
	LOGI("| %10d|", dyReelDist1);
	#ifdef REEL_DIST_PRINT
	//printf("\n dyReelDist1:%d\n", dyReelDist1);
	#endif
	LOGI("------------------------------------------------------");	
	//dyReelDist2
	LOGI("\n[輪帶:r1 r2 隨機數 = 0, r3 隨機數 = 4, r4隨機數 != 0分布]");
	LOGI("\n------------------------------------------------------\n|     number|");
	LOGI("| %10d|", dyReelDist2);
	#ifdef REEL_DIST_PRINT
	//printf("\n dyReelDist2:%d\n", dyReelDist2);
	#endif
	LOGI("------------------------------------------------------");	
	//dyReelDist3
	LOGI("\n[輪帶:r1 r2 r3 隨機數 = 1  r4隨機數 != 1分布]");
	LOGI("\n------------------------------------------------------\n|     number|");
	LOGI("| %10d|", dyReelDist3);
	#ifdef REEL_DIST_PRINT
	//printf("\n dyReelDist3:%d\n", dyReelDist3);
	#endif
	LOGI("------------------------------------------------------");	
	//dyReelDist4
	LOGI("\n[輪帶:r1 r2 隨機數 = 1, r3 隨機數 = 4  r4隨機數 != 1分布]");
	LOGI("\n------------------------------------------------------\n|     number|");
	LOGI("| %10d|", dyReelDist4);
	#ifdef REEL_DIST_PRINT
	//printf("\n dyReelDist4:%d\n", dyReelDist4);
	#endif
	LOGI("------------------------------------------------------");	
	//dyReelDist5
	LOGI("\n[輪帶: r1 r2 r3 隨機數 = 2  r4隨機數 != 2分布]");
	LOGI("\n------------------------------------------------------\n|     number|");
	LOGI("| %10d|", dyReelDist5);
	#ifdef REEL_DIST_PRINT
	//printf("\n dyReelDist5:%d\n", dyReelDist5);
	#endif
	LOGI("------------------------------------------------------");	
	//dyReelDist6
	LOGI("\n[輪帶: r1 r2 隨機數 = 2, r3 隨機數 = 4  r4隨機數 != 2分布]");
	LOGI("\n------------------------------------------------------\n|     number|");
	LOGI("| %10d|", dyReelDist6);
	#ifdef REEL_DIST_PRINT
	//printf("\n dyReelDist6:%d\n", dyReelDist6);
	#endif
	LOGI("------------------------------------------------------");	
	//dyReelDist7
	LOGI("\n[輪帶: r1 r2 r3 隨機數 = 3  r4隨機數 != 3分布]");
	LOGI("\n------------------------------------------------------\n|     number|");
	LOGI("| %10d|", dyReelDist7);
	#ifdef REEL_DIST_PRINT
	//printf("\n dyReelDist7:%d\n", dyReelDist7);
	#endif
	LOGI("------------------------------------------------------");	
	//dyReelDist8
	LOGI("\n[輪帶: r1 r2 隨機數 = 3, r3 隨機數 = 4  r4隨機數 != 3分布]");
	LOGI("\n------------------------------------------------------\n|     number|");
	LOGI("| %10d|", dyReelDist8);
	#ifdef REEL_DIST_PRINT
	//printf("\n dyReelDist8:%d\n", dyReelDist8);
	#endif
	LOGI("------------------------------------------------------");	
	#endif
	
	#ifdef GET_RAND
	LOGI("\n[GETRAND數量分布(確認隨機數是否正常)]");
	char *symbolStr8[TEST_RAND_NUM+1] = {"0", "1", "2", "3", "4"};
	int g;
	LOGI("\n------------------------------------------------------\n| GetRand Dist |  number  |");
	for(g = 0; g <= TEST_RAND_NUM; g++){//print free game SC number dist
		LOGI("| %11s | %10d|", symbolStr8[g], dyRandDist[g]);
		//printf("\n dyRandDist[g]:%d,g:%d\n", dyRandDist[g], g);
	}
	LOGI("------------------------------------------------------");
	#endif
	#ifdef SRAND
	LOGI("\n[SRAND數量分布(確認隨機數是否正常)]");
	char *symbolStr9[TEST_RAND_NUM+1] = {"0", "1", "2", "3", "4"};
	int s;
	LOGI("\n------------------------------------------------------\n| SRand Dist |  number   |");
	for(s = 0; s <= TEST_RAND_NUM; s++){//print free game SC number dist
		LOGI("| %10s | %10d|", symbolStr9[s], dyRandDist1[s]);
		//printf("\n dyRandDist1[s]:%d, s:%d\n", dyRandDist1[s], s);
	}
	LOGI("------------------------------------------------------");
	#endif
	#ifdef WILD_MUL_DIST
	LOGI("\n[WILD倍率分布]");
	char *symbolStr11[6] = {"0", "1", "2", "3", "4", "5"};
	int w;
	LOGI("\n------------------------------------------------------\n| wild mul Dist |  number   |");
	for(w = 1; w <= 5; w++){//print free game SC number dist
		LOGI("| %13s | %10d|", symbolStr11[w], dywildMulDist[w]);
		//printf("\n dywildMulDist[w]:%d, w:%d\n", dywildMulDist[w], w);
	}
	LOGI("------------------------------------------------------");
	#endif		
	
	LOGI("====================");
}

STR_AlgKillFishResult* protocol_mammonKillFish_2(int curBetLineNum, int wantTableId)//curBetLineNum = 40(default), wantTableId = 1
{
	static STR_AlgKillFish val = {0};
	static STR_AlgKillFishResult ret = {0};

	val.times = 1;
	val.lineBet = 10 * val.times;
	val.lineNum = curBetLineNum;//
	val.tableId = wantTableId;//1
	val.buyFreegameScore = 100 * val.lineBet * val.lineNum;//100 * val.lineBet * val.lineNum for buyFreegame, and 0 for base game and free game
	alg_kill_fish(&val, &ret);

	if(val.buyFreegameScore == 0)
		gTotalBet[gMachineIndex] += (double)val.lineNum * val.lineBet / val.times;
	else
		gTotalBet[gMachineIndex] += (double)val.buyFreegameScore / val.times;
	gTotalWin[gMachineIndex] += (double)ret.totalMulti * val.lineBet / val.times;
    if (ret.totalMulti != 0)
    {
        gWinRounds[gMachineIndex]++;
        gEmptyRounds[gMachineIndex] = 0;
        if (ret.prizeType != E_PRIZETYPE_NORMAL)
        {
            if (ret.totalMulti > gMaxMultiFree[gMachineIndex])
            {
                gMaxMultiFree[gMachineIndex] = ret.totalMulti;
            }
            if (ret.totalMulti < gMinMultiFree[gMachineIndex])
            {
                gMinMultiFree[gMachineIndex] = ret.totalMulti;
         
		    }
        }
        else
        {
            if (ret.totalMulti > gMaxMultiNormal[gMachineIndex])
            {
                gMaxMultiNormal[gMachineIndex] = ret.totalMulti;
            }
        }
        if ((gTotalBet[gMachineIndex] - gTotalWin[gMachineIndex]) < gMaxSend[gMachineIndex])
        {
            gMaxSend[gMachineIndex] = gTotalBet[gMachineIndex] - gTotalWin[gMachineIndex];
        }
    }
    else
    {
        gEmptyRounds[gMachineIndex]++;
        if (gEmptyRounds[gMachineIndex] > gMaxEmptyRounds[gMachineIndex])
        {
            gMaxEmptyRounds[gMachineIndex] = gEmptyRounds[gMachineIndex];
        }
    }
    
    if (ret.prizeType != E_PRIZETYPE_NORMAL)
    {
        gFreeRounds[gMachineIndex]++;
        gFreeWin[gMachineIndex] += (double)ret.totalMulti * val.lineBet / val.times;
        gFreeDeviation[gMachineIndex] += ((double)ret.totalMulti - gFreeExp[DIFF]) * ((double)ret.totalMulti - gFreeExp[DIFF]);
    }
    else
    {
        gNormalDeviation[gMachineIndex] += ((double)ret.totalMulti - gNormalExp[DIFF]) * ((double)ret.totalMulti - gNormalExp[DIFF]);
    }

	param_handle(&val, &ret);
	dy_handle(&val, &ret);

	return &ret; 
}

STR_AlgKillFishResult* protocol_mammonKillFish(int curBetLineNum)
{	
	return protocol_mammonKillFish_2(curBetLineNum, TABLE_ID);
}

void protocol_mammonGetData(int needLog)
{
	STR_AlgTestData val;
	STR_AlgTestDataResult ret;

	val.tableId = TABLE_ID;
	alg_get_alg_data(&val, &ret);

	if(needLog)
	{
		LOGI("alg: In=%ld Out=%ld pump=%lf reward:%lf", ret.gRejustIn, ret.gRejustOut, ((double)ret.gRejustIn - ret.gRejustOut) / ret.gRejustIn, (double)ret.gRejustOut / ret.gRejustIn);
		LOGI("cx : In=%f Out=%f pump=%lf reward:%f", gTotalBet[gMachineIndex], gTotalWin[gMachineIndex], (gTotalBet[gMachineIndex] - gTotalWin[gMachineIndex]) / gTotalBet[gMachineIndex], gTotalWin[gMachineIndex]/gTotalBet[gMachineIndex]);
	}
}

void protocol_mammonReset()
{
	STR_AlgReset val;
	STR_AlgResetResult ret;

	val.coin_rate = COIN_RATE;
	val.begin_table_id = 0;
    val.end_table_id = TABLE_ID;
	alg_reset(&val, &ret);
}

void protocol_mammonSetMaxSend()
{
	STR_AlgSetMaxSend val;
	STR_AlgSetMaxSendResult ret;

	val.level = MAX_SEND;
	val.begin_table_id = 0;
    val.end_table_id = TABLE_ID;
	alg_set_max_send(&val, &ret);
}

void protocol_mammonClear()
{
	STR_ClearAllAlgData val;
	STR_ClearAllAlgDataResult ret;

	val.begin_table_id = 0;
    val.end_table_id = TABLE_ID;
	alg_clear_data(&val, &ret);
}

void protocol_mammonSetDiff()
{
	STR_AlgSetDiff val;
	STR_AlgSetDiffResult ret;

	val.diff = gCurDiff;
	val.begin_table_id = 0;
    val.end_table_id = TABLE_ID;

	alg_set_diff(&val, &ret);
}

void test_main_real()
{
	int i;
	char buf[1024] = {0};

    memset(gMinMultiFree, 0xFF, sizeof(gMinMultiFree));
	param_clear();
	dy_clear();
	for (gMachineIndex = 0; gMachineIndex < MACHINE_NUMS; gMachineIndex++)
	{	
		protocol_mammonClear();
		protocol_mammonReset();
		protocol_mammonSetMaxSend();
		protocol_mammonSetDiff();

		gTotalBet[gMachineIndex] = 0;
		gTotalWin[gMachineIndex] = 0;

		for (i = 0; i < TOTAL_ROUNDS; i++)
		{
			if(TOTAL_ROUNDS < 10000)
				printf("-*-*-*-*-*-*-*-test round:%d in test_main_real()\n",i);
			if((TOTAL_ROUNDS >= 10000000) && (i % 10000000 == 0)){
				printf("-*-*-*-*-*-*-*-test round:%d in test_main_real()\n",i);
				snprintf(buf, sizeof(buf), "date");
				system(buf);
			}
			protocol_mammonKillFish(BET_LINE_NUM);//BET_LINE_NUM = 40
			// usleep((int)(0.01 * 1000000));
		}
        LOGI("----------------------------------Machine:%d--------------------------------------", gMachineIndex);
		protocol_mammonGetData(1);
	}

    //LOGI("\n难度\t线数\t抽样号\t总局数\t\t中奖局数\t免费局数\t连续0分局数\t总押\t\t总赢\t\t免费赢\t\t最大负分\t\t普通最高倍率\t免费最低倍率\t免费最高倍率");
    for (i = 0; i < MACHINE_NUMS; i++)
    {
        LOGI("\n难度\t线数\t抽样号\t总局数\t\t中奖局数\t免费局数\t连续0分局数\t总押\t\t总赢\t\t免费赢\t\t最大负分\t\t普通最高倍率\t免费最低倍率\t免费最高倍率");
		printf("\n%d\t%d\t%d\t%d\t%d\t\t%d\t\t%d\t\t%.2f\t%.2f\t%.2f\t%.1f\t\t\t%u\t\t%u\t\t%u",DIFF, BET_LINE_NUM, i, TOTAL_ROUNDS, gWinRounds[i], gFreeRounds[i], gMaxEmptyRounds[i], gTotalBet[i], gTotalWin[i], gFreeWin[i], gMaxSend[i]/COIN_RATE, gMaxMultiNormal[i]/BET_LINE_NUM, gMinMultiFree[i]/BET_LINE_NUM, gMaxMultiFree[i]/BET_LINE_NUM);
		//param_print();//放在for loop裡面會跑出一模一樣的數據
		//dy_print();
		//
		//LOGI("Test finish");
	}
	param_print();
	dy_print();
	
    LOGI("Test finish");
}

/////////////////////////////【对外接口 start】////////////////////////////////
void test_main()
{
	pthread_t temp;
	if (pthread_create(&temp, NULL, (void*)(&test_main_real), NULL))
	{
		LOGE("error: Create test_main_real pthread error\n");
	}
}

/////////////////////////////【对外接口 end】//////////////////////////////////