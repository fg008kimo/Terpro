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
#define TOTAL_ROUNDS    (10000)

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
long dyTotalRound, dyNormalWinRound, dyFreeRound, dyFreeTotalSpins;
double dyTotalBet, dyTotalWin, dyNormalWin, dyFreeWin;
double dyNormalMinMulti, dyNormalMaxMulti, dyFreeMinMulti, dyFreeMaxMulti;
#define DY_PAYOUTDIST_TYPE 3
enum dy_payoutDist_types {DY_PAYOUTDIST_TYPE_TOTAL, DY_PAYOUTDIST_TYPE_NORMAL, DY_PAYOUTDIST_TYPE_FREE};
#define DY_PAYDISTVALUE_LEN 13
const int dyPayoutDistValue[DY_PAYDISTVALUE_LEN] = {1, 5, 10, 20, 30, 50, 100, 200, 300, 400, 500, 1000, 2000};
long dyPayoutDistLQRound[DY_PAYOUTDIST_TYPE][DY_PAYDISTVALUE_LEN];
long dyPayoutDistBetweenRound[DY_PAYOUTDIST_TYPE][DY_PAYDISTVALUE_LEN];
double dyPayoutDistBetweenWin[DY_PAYOUTDIST_TYPE][DY_PAYDISTVALUE_LEN];
#define DY_SYMBOL_PAYOUTDIST_TYPE 4
enum dy_symbol_payoutDist_types {DY_SYMBOL_PAYOUTDIST_TYPE_NORMAL, DY_SYMBOL_PAYOUTDIST_TYPE_FREE, DY_SYMBOL_PAYOUTDIST_TYPE_BUY_NORMAL, DY_SYMBOL_PAYOUTDIST_TYPE_BUY_FREE};
double dySymbolPayoutDistWin[DY_SYMBOL_PAYOUTDIST_TYPE][ICON_NUM+1][BOARD_COLUMN+1];

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
	LOGI("\n=====免费游戏======");
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
	dyTotalRound = dyNormalWinRound = dyFreeRound = dyFreeTotalSpins = 0;
	dyTotalBet = dyTotalWin = dyNormalWin = dyFreeWin = 0;
	dyNormalMinMulti = dyFreeMinMulti = 10000;
	dyNormalMaxMulti = dyFreeMaxMulti = 0;
	memset(dyPayoutDistLQRound, 0, sizeof(dyPayoutDistLQRound));
	memset(dyPayoutDistBetweenRound, 0, sizeof(dyPayoutDistBetweenRound));
	memset(dyPayoutDistBetweenWin, 0, sizeof(dyPayoutDistBetweenWin));
	memset(dySymbolPayoutDistWin, 0, sizeof(dySymbolPayoutDistWin));
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
		u32 freeMulti = ret->totalMulti - ret->normalMulti;
		dyFreeRound++;
		dyFreeTotalSpins += ret->freegameNum;
		dyFreeWin += (double)freeMulti * val->lineBet / val->times;
		dyFreeMinMulti = (dyFreeMinMulti > (double)freeMulti / val->lineNum) ? (double)freeMulti / val->lineNum : dyFreeMinMulti;
		dyFreeMaxMulti = ((double)freeMulti / val->lineNum > dyFreeMaxMulti) ? (double)freeMulti / val->lineNum : dyFreeMaxMulti;
	}

	int type;
	for(type = 0; type < DY_PAYOUTDIST_TYPE; type++)
	{
		int multi, index;
		double win;

		if(type == DY_PAYOUTDIST_TYPE_FREE && ret->prizeType == E_PRIZETYPE_NORMAL)
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
			default:
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
			LOGE("dyPayoutDistBetweenRound error: multi=%d", multi);
		}
	}

	for(type = 0; type < DY_SYMBOL_PAYOUTDIST_TYPE; type++)
	{
		double win;
		int i, j, icon, num;

		if((val->buyFreegameScore == 0 && type >= DY_SYMBOL_PAYOUTDIST_TYPE_BUY_NORMAL) ||
			(val->buyFreegameScore > 0 && type < DY_SYMBOL_PAYOUTDIST_TYPE_BUY_NORMAL))
			continue;

		if((type % 2 > 0) && ret->prizeType == E_PRIZETYPE_NORMAL)
			continue;

		if(type % 2 == 0)
		{
			for(i = 0; i < ret->normalLineNum; i++)
			{
				icon = ret->normalLineInfos[i][1];
				for(num = 0; num <= BOARD_COLUMN; num++)
				{
					if(gIconMulti[icon][num] == ret->normalLineInfos[i][2])//為了知道中獎圖標數量
						break;
				}
				dySymbolPayoutDistWin[type][icon][num] += (double)gIconMulti[icon][num] * val->lineBet / val->times;
			}
		}
		else
		{
			for(i = 0; i < ret->freegameNum; i++)
			{
				for(j = 0; j < ret->freegameLineNum[i]; j++)
				{
					icon = ret->freegameLineInfos[i][j][1];
					for(num = 0; num <= BOARD_COLUMN; num++)
					{
						if(gIconMulti[icon][num] == ret->freegameLineInfos[i][j][2])
							break;
					}
					dySymbolPayoutDistWin[type][icon][num] += (double)gIconMulti[icon][num] * val->lineBet / val->times;
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
	LOGI("一般游戏RTP: %.2f%%", dyNormalWin / dyTotalBet * 100);
	LOGI("免费游戏RTP: %.2f%%", dyFreeWin / dyTotalBet * 100);
	LOGI("游戏整体RTP: %.2f%%\n", dyTotalWin / dyTotalBet * 100);

	LOGI("免费游戏平均赔付倍数: %.2f", dyFreeWin / dyFreeRound / (dyTotalBet / dyTotalRound));
	LOGI("免费游戏平均场次: %.2f", (double)dyFreeTotalSpins / dyFreeRound);
	LOGI("一般游戏触发频率: %.2f%%", (double)dyNormalWinRound / dyTotalRound * 100);
	LOGI("免费游戏触发频率: %.2f%%\n", (double)dyFreeRound / dyTotalRound * 100);

	LOGI("一般游戏最低倍数: %.2f", dyNormalMinMulti);
	LOGI("一般游戏最高倍数: %.2f", dyNormalMaxMulti);
	LOGI("免费游戏最低倍数: %.2f", dyFreeMinMulti);
	LOGI("免费游戏最高倍数: %.2f\n", dyFreeMaxMulti);

	int type;
	LOGI("[开奖分布]");
	char *titles[DY_PAYOUTDIST_TYPE] = {"游戏整体:", "一般游戏:", "免费游戏:"};
	long totalRound[DY_PAYOUTDIST_TYPE] = {dyTotalRound, dyTotalRound, dyFreeRound};
	for(type = 0; type < DY_PAYOUTDIST_TYPE; type++)
	{
		int i;
		LOGI("%s\n---------------------------------------------------------------------------------\n| 赢倍 | 赢分  | HIT RATE   |累计机率| 赢倍区间  | HIT RATE   | 机率   | RTP    |", titles[type]);
		for(i = DY_PAYDISTVALUE_LEN - 1; i >= 0; i--)
		{
			LOGI("| %4d | %5d | %10.2f | %5.2f%% | %4d-%-4d | %10.2f | %5.2f%% | %5.2f%% |", 
				dyPayoutDistValue[i], dyPayoutDistValue[i] * BET_LINE_NUM, (dyPayoutDistLQRound[type][i] == 0 ? 0 : (double)totalRound[type] / dyPayoutDistLQRound[type][i]), (double)dyPayoutDistLQRound[type][i] / totalRound[type] * 100,
				(i == 0 ? 0 : dyPayoutDistValue[i-1]), dyPayoutDistValue[i], (dyPayoutDistBetweenRound[type][i] == 0 ? 0 : (double)totalRound[type] / dyPayoutDistBetweenRound[type][i]), (double)dyPayoutDistBetweenRound[type][i] / totalRound[type] * 100, dyPayoutDistBetweenWin[type][i] / dyTotalBet * 100);
		}
		LOGI("---------------------------------------------------------------------------------");
	}

	LOGI("\n[图标开奖分布]");
	char *titles2[DY_SYMBOL_PAYOUTDIST_TYPE] = {"一般游戏:", "免费游戏:", "一般游戏(购买免费游戏时):", "免费游戏(购买免费游戏时):"};
	char *symbolStr[ICON_NUM+1] = {"", "PIPC1", "PIPC2", "PIPC3", "PIPC4", "SCATTER", "A", "K", "Q", "J", "10", "WILD"};
	for(type = 0; type < DY_SYMBOL_PAYOUTDIST_TYPE; type++)
	{
		int icon;
		LOGI("%s\n------------------------------------------------------\n| SYMBOL| 5of    | 4of    | 3of    | 2of    | 1of    |", titles2[type]);
		for(icon = E_ICON_PIPC1; icon <= E_ICON_WILD; icon++)
		{
			if(icon == E_ICON_SCATTER)
				continue;
			LOGI("| %5s | %5.2f%% | %5.2f%% | %5.2f%% | %5.2f%% | %5.2f%% |", symbolStr[icon],
				dySymbolPayoutDistWin[type][icon][5] / dyTotalBet * 100,
				dySymbolPayoutDistWin[type][icon][4] / dyTotalBet * 100,
				dySymbolPayoutDistWin[type][icon][3] / dyTotalBet * 100,
				dySymbolPayoutDistWin[type][icon][2] / dyTotalBet * 100,
				dySymbolPayoutDistWin[type][icon][1] / dyTotalBet * 100);
		}
		LOGI("------------------------------------------------------");
	}
	LOGI("====================");
}

STR_AlgKillFishResult* protocol_mammonKillFish_2(int curBetLineNum, int wantTableId)//curBetLineNum = 25, wantTableId = 1
{
	static STR_AlgKillFish val = {0};
	static STR_AlgKillFishResult ret = {0};

	val.times = 1;
	val.lineBet = 10 * val.times;
	val.lineNum = curBetLineNum;//curBetLineNum = 25
	val.tableId = wantTableId;//1
	val.buyFreegameScore = 0;//50 * val.lineBet * val.lineNum;
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
			protocol_mammonKillFish(BET_LINE_NUM);//BET_LINE_NUM = 25
			// usleep((int)(0.01 * 1000000));
		}
        LOGI("----------------------------------Machine:%d--------------------------------------", gMachineIndex);
		protocol_mammonGetData(1);
	}

    LOGI("\n难度\t线数\t抽样号\t总局数\t中奖局数\t免费局数\t连续0分局数\t总押\t总赢\t\t免费赢\t\t最大负分\t普通最高倍率\t免费最低倍率\t免费最高倍率");
    for (i = 0; i < MACHINE_NUMS; i++)
    {
        printf("\n%d\t%d\t%d\t%d\t%d\t\t%d\t\t%d\t%.2f\t%.2f\t%.2f\t%.1f\t\t%u\t\t%u\t\t%u",DIFF, BET_LINE_NUM, i, TOTAL_ROUNDS, gWinRounds[i], gFreeRounds[i], gMaxEmptyRounds[i], gTotalBet[i], gTotalWin[i], gFreeWin[i], gMaxSend[i]/COIN_RATE, gMaxMultiNormal[i]/BET_LINE_NUM, gMinMultiFree[i]/BET_LINE_NUM, gMaxMultiFree[i]/BET_LINE_NUM);
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