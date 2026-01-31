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

#define TOTAL_BET       20
#define TABLE_ID        1
#define MACHINE_NUMS    1
#define TOTAL_ROUNDS    (1000000)

uint32_t gWinRounds[MACHINE_NUMS];
uint32_t gFreeRounds[MACHINE_NUMS];
uint32_t gEmptyRounds[MACHINE_NUMS];
uint32_t gMaxEmptyRounds[MACHINE_NUMS];
double gTotalBet[MACHINE_NUMS];
double gTotalWin[MACHINE_NUMS];
double gFreeWin[MACHINE_NUMS];
double gMaxMultiNormal[MACHINE_NUMS];
double gMaxMultiFree[MACHINE_NUMS];
double gMinMultiFree[MACHINE_NUMS] = {0xFFFFFFFF};
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
#define DY_PAYDISTVALUE_LEN 15
const int dyPayoutDistValue[DY_PAYDISTVALUE_LEN] = {1, 5, 10, 20, 30, 50, 100, 200, 300, 400, 500, 1000, 2000, 5000, 10000};
long dyPayoutDistLQRound[DY_PAYOUTDIST_TYPE][DY_PAYDISTVALUE_LEN];
long dyPayoutDistBetweenRound[DY_PAYOUTDIST_TYPE][DY_PAYDISTVALUE_LEN];
double dyPayoutDistBetweenWin[DY_PAYOUTDIST_TYPE][DY_PAYDISTVALUE_LEN];
#define DY_SYMBOL_PAYOUTDIST_TYPE 4
enum dy_symbol_payoutDist_types {DY_SYMBOL_PAYOUTDIST_TYPE_NORMAL, DY_SYMBOL_PAYOUTDIST_TYPE_FREE, DY_SYMBOL_PAYOUTDIST_TYPE_BUY_NORMAL, DY_SYMBOL_PAYOUTDIST_TYPE_BUY_FREE};
double dySymbolPayoutDistWin[DY_SYMBOL_PAYOUTDIST_TYPE][ICON_NUM+1][5];
#define DY_TUMBLE_DIST_TYPE 4
enum dy_tumble_dist_types {DY_TUMBLE_DIST_TYPE_NORMAL, DY_TUMBLE_DIST_TYPE_FREE, DY_TUMBLE_DIST_TYPE_BUY_NORMAL, DY_TUMBLE_DIST_TYPE_BUY_FREE};
long dyCascadingCount[DY_TUMBLE_DIST_TYPE][21];
long dyFreeGameCount[2][91];
long dyMulIconCount[DY_TUMBLE_DIST_TYPE][21];

int calMulIconTotalMulti(int *mulIconMultis, int mulIconNum)
{
	int totalMulti = 0, i;
	for(i = 0; i < mulIconNum; i++)
		totalMulti += mulIconMultis[i];
	totalMulti = (totalMulti == 0 ? 1 : totalMulti);
	return totalMulti;
}

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
	double totalMulti = (double)ret->totalMulti / ICON_MULTI_MAGNIFY;
	freeGameTotalMulti += totalMulti;
	if(ret->prizeType == E_PRIZETYPE_FREEGAME)
	{
		freeGameFreeRound++;
		freeGameCurSeqNonRound = 0;
		freeGameFreeMulti += totalMulti;
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
	LOGI("平均倍率：	      %f", freeGameFreeMulti/freeGameFreeRound);
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
	memset(dyCascadingCount, 0, sizeof(dyCascadingCount));
	memset(dyFreeGameCount, 0, sizeof(dyFreeGameCount));
	memset(dyMulIconCount, 0, sizeof(dyMulIconCount));
}

int dy_getSymbolPayoutDistIndex(int icon, int num)
{
	if(icon == E_ICON_SCATTER)
	{
		if(num == 4)
			return 0;
		else if(num == 5)
			return 1;
		else
			return 2;
	}
	else
	{
		if(num == 8)
			return 0;
		else if(num == 9)
			return 1;
		else if(num == 10)
			return 2;
		else if(num == 11)
			return 3;
		else
			return 4;
	}
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
		dyTotalBet += (double)val->totalBetScore / val->times;
	dyTotalWin += (double)ret->totalMulti / ICON_MULTI_MAGNIFY * val->totalBetScore / val->times;
	u32 normalMulti = 0, i;
	for(i = 0; i < ret->normalCascadeNum; i++)
		normalMulti += ret->normalMultis[i] * calMulIconTotalMulti(ret->normalMulIconMultis, ret->normalMulIconNum);
	if(normalMulti > 0)
	{
		dyNormalWinRound++;
		dyNormalWin += (double)normalMulti / ICON_MULTI_MAGNIFY * val->totalBetScore / val->times;
		dyNormalMinMulti = (dyNormalMinMulti > (double)normalMulti / ICON_MULTI_MAGNIFY) ? (double)normalMulti / ICON_MULTI_MAGNIFY : dyNormalMinMulti;
		dyNormalMaxMulti = ((double)normalMulti / ICON_MULTI_MAGNIFY > dyNormalMaxMulti) ? (double)normalMulti / ICON_MULTI_MAGNIFY : dyNormalMaxMulti;
	}
	if(ret->prizeType == E_PRIZETYPE_FREEGAME)
	{
		u32 freeMulti = ret->totalMulti - normalMulti;
		dyFreeRound++;
		dyFreeTotalSpins += ret->freegameNum;
		dyFreeWin += (double)freeMulti / ICON_MULTI_MAGNIFY * val->totalBetScore / val->times;
		dyFreeMinMulti = (dyFreeMinMulti > (double)freeMulti / ICON_MULTI_MAGNIFY) ? (double)freeMulti / ICON_MULTI_MAGNIFY : dyFreeMinMulti;
		dyFreeMaxMulti = ((double)freeMulti / ICON_MULTI_MAGNIFY > dyFreeMaxMulti) ? (double)freeMulti / ICON_MULTI_MAGNIFY : dyFreeMaxMulti;
	}

	int type;
	for(type = 0; type < DY_PAYOUTDIST_TYPE; type++)
	{
		int index;
		double multi, win, buyFreeGameMulti;

		if(type == DY_PAYOUTDIST_TYPE_FREE && ret->prizeType == E_PRIZETYPE_NORMAL)
			continue;

		buyFreeGameMulti = (val->buyFreegameScore > 0) ? (double)val->buyFreegameScore / val->totalBetScore : 1;
		switch(type)
		{
			case DY_PAYOUTDIST_TYPE_TOTAL:
				multi = (double)ret->totalMulti / ICON_MULTI_MAGNIFY / buyFreeGameMulti;
				break;
			case DY_PAYOUTDIST_TYPE_NORMAL:
				multi = (double)normalMulti / ICON_MULTI_MAGNIFY / buyFreeGameMulti;
				break;
			default:
				multi = (double)(ret->totalMulti - normalMulti) / ICON_MULTI_MAGNIFY / buyFreeGameMulti;
				break;
		}
		win = multi * buyFreeGameMulti * val->totalBetScore / val->times;

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
			LOGE("dyPayoutDistBetweenRound error: multi=%f", multi);
		}
	}

	for(type = 0; type < DY_SYMBOL_PAYOUTDIST_TYPE; type++)
	{
		int cascadeNum, rewardIconTypes, freegameNum;

		if((val->buyFreegameScore == 0 && type >= DY_SYMBOL_PAYOUTDIST_TYPE_BUY_NORMAL) ||
			(val->buyFreegameScore > 0 && type < DY_SYMBOL_PAYOUTDIST_TYPE_BUY_NORMAL))
			continue;

		if((type % 2 > 0) && ret->prizeType == E_PRIZETYPE_NORMAL)
			continue;

		if(type % 2 == 0)
		{
			for(cascadeNum = 0; cascadeNum < ret->normalCascadeNum; cascadeNum++)
			{
				for(rewardIconTypes = 0; rewardIconTypes < ret->normalRewardIconTypes[cascadeNum]; rewardIconTypes++)
				{
					int *rewardInfos = ret->normalRewardInfos[cascadeNum][rewardIconTypes];
					dySymbolPayoutDistWin[type][rewardInfos[0]][dy_getSymbolPayoutDistIndex(rewardInfos[0], rewardInfos[1])] += (double)rewardInfos[2] * calMulIconTotalMulti(ret->normalMulIconMultis, ret->normalMulIconNum) / ICON_MULTI_MAGNIFY * val->totalBetScore / val->times;
				}
			}
		}
		else
		{
			for(freegameNum = 0; freegameNum < ret->freegameNum; freegameNum++)
			{
				for(cascadeNum = 0; cascadeNum < ret->freegameCascadeNums[freegameNum]; cascadeNum++)
				{
					for(rewardIconTypes = 0; rewardIconTypes < ret->freegameRewardIconTypes[freegameNum][cascadeNum]; rewardIconTypes++)
					{
						int *rewardInfos = ret->freegameRewardInfos[freegameNum][cascadeNum][rewardIconTypes];
						dySymbolPayoutDistWin[type][rewardInfos[0]][dy_getSymbolPayoutDistIndex(rewardInfos[0], rewardInfos[1])] += (double)rewardInfos[2] * calMulIconTotalMulti(ret->freegameMulIconMultis[freegameNum], ret->freegameMulIconNums[freegameNum]) / ICON_MULTI_MAGNIFY * val->totalBetScore / val->times;
					}
				}
			}
		}
	}

	for(type = 0; type < DY_TUMBLE_DIST_TYPE; type++)
	{
		if((val->buyFreegameScore == 0 && type >= DY_TUMBLE_DIST_TYPE_BUY_NORMAL) ||
			(val->buyFreegameScore > 0 && type < DY_TUMBLE_DIST_TYPE_BUY_NORMAL))
			continue;

		if((type % 2 > 0) && ret->prizeType == E_PRIZETYPE_NORMAL)
			continue;

		if(type % 2 == 0)
		{
			dyCascadingCount[type][ret->normalCascadeNum-1]++;
			dyMulIconCount[type][ret->normalMulIconNum]++;
		}
		else
		{
			int num;
			for(num = 0; num < ret->freegameNum; num++)
			{
				dyCascadingCount[type][ret->freegameCascadeNums[num]-1]++;
				dyMulIconCount[type][ret->freegameMulIconNums[num]]++;
			}
			dyFreeGameCount[type/2][ret->freegameNum]++;
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
				dyPayoutDistValue[i], dyPayoutDistValue[i] * TOTAL_BET, (dyPayoutDistLQRound[type][i] == 0 ? 0 : (double)totalRound[type] / dyPayoutDistLQRound[type][i]), (double)dyPayoutDistLQRound[type][i] / totalRound[type] * 100,
				(i == 0 ? 0 : dyPayoutDistValue[i-1]), dyPayoutDistValue[i], (dyPayoutDistBetweenRound[type][i] == 0 ? 0 : (double)totalRound[type] / dyPayoutDistBetweenRound[type][i]), (double)dyPayoutDistBetweenRound[type][i] / totalRound[type] * 100, dyPayoutDistBetweenWin[type][i] / dyTotalBet * 100);
		}
		LOGI("---------------------------------------------------------------------------------");
	}

	LOGI("\n[图标开奖分布]");
	char *titles2[DY_SYMBOL_PAYOUTDIST_TYPE] = {"一般游戏:", "免费游戏:", "一般游戏(购买免费游戏时):", "免费游戏(购买免费游戏时):"};
	char *symbolStr[ICON_NUM+1] = {"", "PIPC1", "PIPC2", "PIPC3", "PIPC4", "A", "K", "Q", "J", "10", "SC"};
	for(type = 0; type < DY_SYMBOL_PAYOUTDIST_TYPE; type++)
	{
		int icon;
		LOGI("%s\n------------------------------------------------------\n| SYMBOL| 8of    | 9of    | 10of   | 11of   | 12+of  |", titles2[type]);
		for(icon = E_ICON_PIPC1; icon < E_ICON_MUL_BASE; icon++)
		{
			LOGI("| %5s | %5.2f%% | %5.2f%% | %5.2f%% | %5.2f%% | %5.2f%% |", symbolStr[icon],
				dySymbolPayoutDistWin[type][icon][0] / dyTotalBet * 100,
				dySymbolPayoutDistWin[type][icon][1] / dyTotalBet * 100,
				dySymbolPayoutDistWin[type][icon][2] / dyTotalBet * 100,
				dySymbolPayoutDistWin[type][icon][3] / dyTotalBet * 100,
				dySymbolPayoutDistWin[type][icon][4] / dyTotalBet * 100);
		}
		LOGI("------------------------------------------------------\n|       |   4of  |   5of  |  6+of  |");
		LOGI("|    SC | %5.2f%% | %5.2f%% | %5.2f%% |", dySymbolPayoutDistWin[type][E_ICON_SCATTER][0] / dyTotalBet * 100,
			dySymbolPayoutDistWin[type][E_ICON_SCATTER][1] / dyTotalBet * 100, dySymbolPayoutDistWin[type][E_ICON_SCATTER][2] / dyTotalBet * 100);
		LOGI("------------------------------------------------------");
	}

	LOGI("\n[翻滚分布]");
	char *titles3[DY_TUMBLE_DIST_TYPE] = {"一般游戏:", "免费游戏:", "一般游戏(购买免费游戏时):", "免费游戏(购买免费游戏时):"};
	long totalRound3[DY_TUMBLE_DIST_TYPE] = {dyTotalRound, dyFreeTotalSpins, dyTotalRound, dyFreeTotalSpins};
	for(type = 0; type < DY_TUMBLE_DIST_TYPE; type++)
	{
		int i;
		LOGI("%s\n----------------------------------------\n| Cascading Count | Frequency | Prob   |", titles3[type]);
		for(i = 0; i <= 20; i++)
			LOGI("|       %2d        | %9ld | %5.2f%% |", i, dyCascadingCount[type][i], (double)dyCascadingCount[type][i] / totalRound3[type] * 100);

		LOGI("%s\n----------------------------------------\n| Multiplier Count | Frequency | Prob   |", titles3[type]);
		for(i = 0; i <= 20; i++)
			LOGI("|       %2d         | %9ld | %5.2f%% |", i, dyMulIconCount[type][i], (double)dyMulIconCount[type][i] / totalRound3[type] * 100);

		if(type % 2 > 0) //免费游戏
		{
			LOGI("%s\n----------------------------------------\n| Free Game Count | Frequency | Prob   |", titles3[type]);
			for(i = 10; i <= 90; i += 5)
				LOGI("|       %2d        | %9ld | %5.2f%% |", i, dyFreeGameCount[type/2][i], (double)dyFreeGameCount[type/2][i] / dyFreeRound * 100);
		}
	}
	LOGI("====================");
}

STR_AlgKillFishResult* protocol_mammonKillFish_2(int wantTableId)
{
	static STR_AlgKillFish val = {0};
	static STR_AlgKillFishResult ret = {0};

	val.times = 1;
	val.totalBetScore = TOTAL_BET * val.times;
	val.tableId = wantTableId;
	val.buyFreegameScore = 0;//100 * val.totalBetScore;
	alg_kill_fish(&val, &ret);

	if(val.buyFreegameScore == 0)
		gTotalBet[gMachineIndex] += (double)val.totalBetScore / val.times;
	else
		gTotalBet[gMachineIndex] += (double)val.buyFreegameScore / val.times;
	double totalMulti = (double)ret.totalMulti / ICON_MULTI_MAGNIFY;
	gTotalWin[gMachineIndex] += totalMulti * val.totalBetScore / val.times;
    if (ret.totalMulti != 0)
    {
        gWinRounds[gMachineIndex]++;
        gEmptyRounds[gMachineIndex] = 0;
        if (ret.prizeType != E_PRIZETYPE_NORMAL)
        {
            if (totalMulti > gMaxMultiFree[gMachineIndex])
            {
                gMaxMultiFree[gMachineIndex] = totalMulti;
            }
            if (totalMulti < gMinMultiFree[gMachineIndex])
            {
                gMinMultiFree[gMachineIndex] = totalMulti;
         
		    }
        }
        else
        {
            if (totalMulti > gMaxMultiNormal[gMachineIndex])
            {
                gMaxMultiNormal[gMachineIndex] = totalMulti;
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
        gFreeWin[gMachineIndex] += totalMulti * val.totalBetScore / val.times;
        gFreeDeviation[gMachineIndex] += (totalMulti - gFreeExp[DIFF]) * (totalMulti - gFreeExp[DIFF]);
    }
    else
    {
        gNormalDeviation[gMachineIndex] += (totalMulti - gNormalExp[DIFF]) * (totalMulti - gNormalExp[DIFF]);
    }

	param_handle(&val, &ret);
	dy_handle(&val, &ret);

	return &ret; 
}

STR_AlgKillFishResult* protocol_mammonKillFish()
{	
	return protocol_mammonKillFish_2(TABLE_ID);
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
			protocol_mammonKillFish();
			// usleep((int)(0.01 * 1000000));
		}
        LOGI("----------------------------------Machine:%d--------------------------------------", gMachineIndex);
		protocol_mammonGetData(1);
	}

    LOGI("\n难度\t抽样号\t总局数\t中奖局数\t免费局数\t连续0分局数\t总押\t总赢\t\t免费赢\t\t最大负分\t普通最高倍率\t免费最低倍率\t免费最高倍率");
    for (i = 0; i < MACHINE_NUMS; i++)
    {
        printf("\n%d\t%d\t%d\t%d\t\t%d\t\t%d\t%.2f\t%.2f\t%.2f\t%.1f\t\t%.2f\t\t%.2f\t\t%.2f",DIFF, i, TOTAL_ROUNDS, gWinRounds[i], gFreeRounds[i], gMaxEmptyRounds[i], gTotalBet[i], gTotalWin[i], gFreeWin[i], gMaxSend[i]/COIN_RATE, gMaxMultiNormal[i], gMinMultiFree[i], gMaxMultiFree[i]);
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