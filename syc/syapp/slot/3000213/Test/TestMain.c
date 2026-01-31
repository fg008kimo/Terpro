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
#define TOTAL_ROUNDS    (100000000)

uint32_t gWinRounds[MACHINE_NUMS];
uint32_t gEmptyRounds[MACHINE_NUMS];
uint32_t gMaxEmptyRounds[MACHINE_NUMS];
double gTotalBet[MACHINE_NUMS];
double gTotalWin[MACHINE_NUMS];
uint32_t gMaxMultiTotal[MACHINE_NUMS];
double gMaxSend[MACHINE_NUMS];
double gTotalDeviation[MACHINE_NUMS];//方差
const double gTotalExp[10] = {6.2685,6.237,6.2055,6.174,6.1425,6.111,6.0795,6.048,6.0165,5.985};

static uint32_t gMachineIndex = 0;
static uint32_t gCurDiff = DIFF;

//dongyuan指标
#define CHECK_DONGYUAN
long dyTotalRound, dyTotalWinRound;
double dyTotalBet, dyTotalWin;
double dyTotalMinMulti, dyTotalMaxMulti;
#define DY_PAYOUTDIST_TYPE 1
enum dy_payoutDist_types {DY_PAYOUTDIST_TYPE_TOTAL};
#define DY_PAYDISTVALUE_LEN 13
const int dyPayoutDistValue[DY_PAYDISTVALUE_LEN] = {1, 5, 10, 20, 30, 50, 100, 200, 300, 400, 500, 1000, 2000};
long dyPayoutDistLQRound[DY_PAYOUTDIST_TYPE][DY_PAYDISTVALUE_LEN];
long dyPayoutDistBetweenRound[DY_PAYOUTDIST_TYPE][DY_PAYDISTVALUE_LEN];
double dyPayoutDistBetweenWin[DY_PAYOUTDIST_TYPE][DY_PAYDISTVALUE_LEN];
#define DY_SYMBOL_PAYOUTDIST_TYPE 1
enum dy_symbol_payoutDist_types {DY_SYMBOL_PAYOUTDIST_TYPE_NORMAL};
double dySymbolPayoutDistWin[DY_SYMBOL_PAYOUTDIST_TYPE][ICON_NUM+1];

void dy_clear()
{
#ifndef CHECK_DONGYUAN
	return;
#endif
	dyTotalRound = dyTotalWinRound = 0;
	dyTotalBet = dyTotalWin = 0;
	dyTotalMinMulti = 10000;
	dyTotalMaxMulti = 0;
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
	dyTotalBet += (double)val->lineNum * val->lineBet / val->times;
	dyTotalWin += (double)ret->totalMulti * val->lineBet / val->times;
	if(ret->totalMulti > 0)
	{
		dyTotalWinRound++;
		dyTotalMinMulti = (dyTotalMinMulti > (double)ret->totalMulti / val->lineNum) ? (double)ret->totalMulti / val->lineNum : dyTotalMinMulti;
		dyTotalMaxMulti = ((double)ret->totalMulti / val->lineNum > dyTotalMaxMulti) ? (double)ret->totalMulti / val->lineNum : dyTotalMaxMulti;
	}

	int type;
	for(type = 0; type < DY_PAYOUTDIST_TYPE; type++)
	{
		int multi, index;
		double win;
		
		switch(type)
		{
			case DY_PAYOUTDIST_TYPE_TOTAL:
				multi = ret->totalMulti / val->lineNum;
				win = (double)ret->totalMulti * val->lineBet / val->times;
				break;
			default:
				multi = ret->totalMulti / val->lineNum;
				win = (double)ret->totalMulti * val->lineBet / val->times;
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


		if(type % 2 == 0)
		{
			for(i = 0; i < ret->normalLineNum; i++)
			{
				icon = ret->normalLineInfos[i][1];
				dySymbolPayoutDistWin[type][icon] += (double)ret->normalLineInfos[i][2] * val->lineBet / val->times;
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
	LOGI("游戏整体RTP: %.2f%%\n", dyTotalWin / dyTotalBet * 100);

	LOGI("游戏贏錢率: %.2f%%", (double)dyTotalWinRound / dyTotalRound * 100);

	LOGI("游戏最低倍数: %.2f", dyTotalMinMulti);
	LOGI("游戏最高倍数: %.2f", dyTotalMaxMulti);

	int type;
	LOGI("[开奖分布]");
	char *titles[DY_PAYOUTDIST_TYPE] = {"游戏整体:"};
	long totalRound[DY_PAYOUTDIST_TYPE] = {dyTotalRound};
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
	char *titles2[DY_SYMBOL_PAYOUTDIST_TYPE] = {"一般游戏:"};
	char *symbolStr[ICON_NUM+1] = {"", "PIPC1", "PIPC2", "PIPC3", "PIPC4", "PIPC5", "PIPC6", "PIPC7", "WILD"};
	for(type = 0; type < DY_SYMBOL_PAYOUTDIST_TYPE; type++)
	{
		int icon;
		LOGI("%s\n------------------------------------------------------\n| SYMBOL|    RTP   |", titles2[type]);
		for(icon = E_ICON_PIPC1; icon <= E_ICON_WILD; icon++)
		{
			LOGI("| %5s | %7.2f%% |", symbolStr[icon],
				dySymbolPayoutDistWin[type][icon] / dyTotalBet * 100);
		}
		LOGI("------------------------------------------------------");
	}
	LOGI("====================");
}

STR_AlgKillFishResult* protocol_mammonKillFish_2(int curBetLineNum, int wantTableId)
{
	static STR_AlgKillFish val = {0};
	static STR_AlgKillFishResult ret = {0};

	val.times = 1;
	val.lineBet = 10 * val.times;
	val.lineNum = curBetLineNum;
	val.tableId = wantTableId;
	val.blackDiff = 4;
	val.blackDiffType = 5;
	val.uid = 0;
	val.playerDiff = 9;//0-9	
	alg_kill_fish(&val, &ret);

	gTotalBet[gMachineIndex] += (double)val.lineNum * val.lineBet / val.times;

	gTotalWin[gMachineIndex] += (double)ret.totalMulti * val.lineBet / val.times;
    if (ret.totalMulti != 0)
    {
        gWinRounds[gMachineIndex]++;
        gEmptyRounds[gMachineIndex] = 0;
        
        if (ret.totalMulti > gMaxMultiTotal[gMachineIndex])
        {
            gMaxMultiTotal[gMachineIndex] = ret.totalMulti;
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
    

    gTotalDeviation[gMachineIndex] += ((double)ret.totalMulti - gTotalExp[DIFF]) * ((double)ret.totalMulti - gTotalExp[DIFF]);

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
			if(TOTAL_ROUNDS < 1000000)
				printf("-*-*-*-*-*-*-*-test round:%d in test_main_real()\n",i);
			if((TOTAL_ROUNDS >= 1000000) && (i % 1000000 == 0)){
				printf("-*-*-*-*-*-*-*-test round:%d in test_main_real()\n",i);
				snprintf(buf, sizeof(buf), "date");
				system(buf);
			}
			protocol_mammonKillFish(BET_LINE_NUM);
		}
        LOGI("----------------------------------Machine:%d--------------------------------------", gMachineIndex);
		protocol_mammonGetData(1);
	}

    LOGI("\n难度\t线数\t抽样号\t总局数\t中奖局数\t连续0分局数\t总押\t总赢\t\t最大负分\t普通最高倍率");
    for (i = 0; i < MACHINE_NUMS; i++)
    {
        printf("\n%d\t%d\t%d\t%d\t%d\t\t%d\t%.2f\t%.2f\t%.1f\t\t%u",DIFF, BET_LINE_NUM, i, TOTAL_ROUNDS, gWinRounds[i], gMaxEmptyRounds[i], gTotalBet[i], gTotalWin[i], gMaxSend[i]/COIN_RATE, gMaxMultiTotal[i]/BET_LINE_NUM);
    }
	
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