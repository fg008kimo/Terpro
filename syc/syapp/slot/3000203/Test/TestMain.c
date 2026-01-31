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
#define BUYFREEGAME     1
#define TOTAL_ROUNDS    (1000000)

uint32_t gWinRounds[MACHINE_NUMS];
uint32_t gFreeRounds[MACHINE_NUMS], gSpecialRounds[MACHINE_NUMS];
uint32_t gEmptyRounds[MACHINE_NUMS];
uint32_t gMaxEmptyRounds[MACHINE_NUMS];
double gTotalBet[MACHINE_NUMS];
double gTotalWin[MACHINE_NUMS];
double gFreeWin[MACHINE_NUMS], gSpecialWin[MACHINE_NUMS];
uint32_t gMaxMultiNormal[MACHINE_NUMS];
uint32_t gMaxMultiFree[MACHINE_NUMS];
uint32_t gMinMultiFree[MACHINE_NUMS] = {0xFFFFFFFF};
uint32_t gMaxMultiSpecial[MACHINE_NUMS];
uint32_t gMinMultiSpecial[MACHINE_NUMS] = {0xFFFFFFFF};
double gMaxSend[MACHINE_NUMS];
double gFreeDeviation[MACHINE_NUMS];//方差
const double gNormalExp[10] = {6.2685,6.237,6.2055,6.174,6.1425,6.111,6.0795,6.048,6.0165,5.985};
double gNormalDeviation[MACHINE_NUMS];//方差
const double gFreeExp[10] = {537.3,534.6,531.9,529.2,526.5,523.8,521.1,518.4,515.7,513};

static uint32_t gMachineIndex = 0;
static uint32_t gCurDiff = DIFF;

//免费游戏
#define CHECK_FREEGAME
s32 freeGameTotalRound, freeGameFreeRound, freeGameCurSeqNonRound, freeGameMaxSeqNonRound;
double freeGameTotalMulti, freeGameFreeMulti;
s64 freeGameNumTotal, freeGameNumMax;

//特殊游戏
// #define CHECK_SPECIAL
s32 specialTotalRound, specialRound, specialCurSeqNonRound, specialMaxSeqNonRound;
double specialTotalMulti, specialMulti;
s64 specialNumTotal, specialNumMax;

//猫和老鼠指标
#define CHECK_TOMJERRY	
long frTotalRound, frNormalWinRound, frFreeRound, frFreeTotalSpins, frSpecialRound, frSpecialTotalSpins;
double frTotalBet, frTotalWin, frNormalWin, frFreeWin, frSpecialWin;
double frNormalMinMulti, frNormalMaxMulti, frFreeMinMulti, frFreeMaxMulti, frSpecialMinMulti, frSpecialMaxMulti;
#define FR_PAYOUTDIST_TYPE 3
enum fr_payoutDist_types {FR_PAYOUTDIST_TYPE_TOTAL, FR_PAYOUTDIST_TYPE_NORMAL, FR_PAYOUTDIST_TYPE_FREE};
#define FR_PAYDISTVALUE_LEN 17
const int frPayoutDistValue[FR_PAYDISTVALUE_LEN] = {0, 1, 5, 10, 20, 30, 50, 100, 200, 300, 400, 500, 1000, 2000, 5000, 10000, 12500};   //赢倍
long frPayoutDistLQRound[FR_PAYDISTVALUE_LEN][FR_PAYDISTVALUE_LEN];         //相应的倍率区间出现的局数
long frPayoutDistBetweenRound[FR_PAYDISTVALUE_LEN][FR_PAYDISTVALUE_LEN];    //赢倍区间
int frPayoutDistBetweenWin[FR_PAYDISTVALUE_LEN][FR_PAYDISTVALUE_LEN];    //赢倍区间的分数总和
#define FR_SYMBOL_PAYOUTDIST_TYPE 3
enum fr_symbol_payoutDist_types {FR_SYMBOL_PAYOUTDIST_TYPE_NORMAL, FR_SYMBOL_PAYOUTDIST_TYPE_FREE, FR_SYMBOL_PAYOUTDIST_TYPE_BUY_FREE};
double frSymbolPayoutDistWin[FR_SYMBOL_PAYOUTDIST_TYPE][ICON_NUM][BOARD_COLUMN+1];   //记录线的得分情况

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

void special_clean()
{
#ifdef CHECK_SPECIAL
	specialTotalRound = 0;
	specialRound = 0;
	specialCurSeqNonRound = 0;
	specialMaxSeqNonRound = 0;
	specialTotalMulti = 0;
	specialMulti = 0;
	specialNumTotal = 0;
	specialNumMax = 0;
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

void special_handle(STR_AlgKillFish* val, STR_AlgKillFishResult* ret)
{
#ifdef CHECK_SPECIAL
	specialTotalRound++;
	specialTotalMulti += ret->totalMulti;
	if(ret->prizeType == E_PRIZETYPE_SPECIAL)
	{
		specialRound++;
		specialCurSeqNonRound = 0;
		specialMulti += ret->totalMulti;
		specialNumTotal += ret->specialNum;
		if(ret->specialNum > specialNumMax)
			specialNumMax = ret->specialNum;
	}
	else
	{
		specialCurSeqNonRound++;
		if(specialCurSeqNonRound > specialMaxSeqNonRound)
			specialMaxSeqNonRound = specialCurSeqNonRound;
	}
#endif
}

void freeGame_print()
{
#ifdef CHECK_FREEGAME
	LOGI("\n======免费游戏======");
	LOGI("平均几局出一次：      %f", (double)freeGameTotalRound/freeGameFreeRound);
	LOGI("最大连续不中局数：    %d", freeGameMaxSeqNonRound);
	LOGI("平均倍率：	      %f", freeGameFreeMulti/freeGameFreeRound/BET_LINE_NUM);
	LOGI("出分占比：	      %f%%", freeGameFreeMulti*100/freeGameTotalMulti);
	LOGI("内部平均局数：	      %f", (double)freeGameNumTotal/freeGameFreeRound);
	LOGI("内部最大局数：	      %ld", freeGameNumMax);
	LOGI("====================");
#endif
}

void special_print()
{
#ifdef CHECK_SPECIAL
	LOGI("\n======特殊游戏======");
	LOGI("平均几局出一次：      %f", (double)specialTotalRound/specialRound);
	LOGI("最大连续不中局数：    %d", specialMaxSeqNonRound);
	LOGI("平均倍率：	      %f", specialMulti/specialRound/BET_LINE_NUM);
	LOGI("出分占比：	      %f%%", specialMulti*100/specialTotalMulti);
	LOGI("内部平均局数：	      %f", (double)specialNumTotal/specialRound);
	LOGI("内部最大局数：	      %ld", specialNumMax);
	LOGI("====================");
#endif
}

void param_clear()
{
	freeGame_clean();
	special_clean();
}

void param_handle(STR_AlgKillFish* val, STR_AlgKillFishResult* ret)
{
	freeGame_handle(val, ret);
	special_handle(val, ret);
}

void param_print()
{
	freeGame_print();
	special_print();
}

void fr_clear()
{
#ifndef CHECK_TOMJERRY
	return;
#endif
	frTotalRound = frNormalWinRound = frFreeRound = frFreeTotalSpins = frSpecialRound = frSpecialTotalSpins = 0;
	frTotalBet = frTotalWin = frNormalWin = frSpecialWin = frFreeWin = 0;
	frNormalMinMulti = frFreeMinMulti = frSpecialMinMulti = 10000;
	frNormalMaxMulti = frFreeMaxMulti = frSpecialMaxMulti = 0;
	memset(frPayoutDistLQRound, 0, sizeof(frPayoutDistLQRound));
	memset(frPayoutDistBetweenRound, 0, sizeof(frPayoutDistBetweenRound));
	memset(frPayoutDistBetweenWin, 0, sizeof(frPayoutDistBetweenWin));
	memset(frSymbolPayoutDistWin, 0, sizeof(frSymbolPayoutDistWin));
}

void fr_handle(STR_AlgKillFish* val, STR_AlgKillFishResult* ret)
{
#ifndef CHECK_TOMJERRY
	return;
#endif
	frTotalRound++;
	if(BUYFREEGAME)
		frTotalBet += (double)val->buyFreegameScore / val->times;
	else
		frTotalBet += (double)val->lineNum * val->lineBet / val->times;
	frTotalWin += (double)ret->totalMulti * val->lineBet / val->times;

	if(ret->normalMulti > 0)
	{
		frNormalWinRound++;
		frNormalWin += (double)ret->normalMulti * val->lineBet / val->times;
		frNormalMinMulti = (frNormalMinMulti > (double)ret->normalMulti / val->lineNum) ? (double)ret->normalMulti / val->lineNum : frNormalMinMulti;
		frNormalMaxMulti = ((double)ret->normalMulti / val->lineNum > frNormalMaxMulti) ? (double)ret->normalMulti / val->lineNum : frNormalMaxMulti;
	}
	if(ret->prizeType == E_PRIZETYPE_FREEGAME)
	{
		u32 freeMulti = ret->totalMulti - ret->normalMulti;
		frFreeRound++;
		frFreeTotalSpins += ret->freegameNum;
		frFreeWin += (double)freeMulti * val->lineBet / val->times;
		frFreeMinMulti = (frFreeMinMulti > (double)freeMulti / val->lineNum) ? (double)freeMulti / val->lineNum : frFreeMinMulti;
		frFreeMaxMulti = ((double)freeMulti / val->lineNum > frFreeMaxMulti) ? (double)freeMulti / val->lineNum : frFreeMaxMulti;
	}

	int type;
	for(type = 0; type < FR_PAYOUTDIST_TYPE; type++)
	{
		int multi, index;
		double win;

		switch(type)
		{
			case FR_PAYOUTDIST_TYPE_TOTAL:
				multi = ret->totalMulti / val->lineNum;
				win = (double)ret->totalMulti * val->lineBet / val->times;
				break;
			case FR_PAYOUTDIST_TYPE_NORMAL:
				multi = ret->normalMulti / val->lineNum;
				win = (double)ret->normalMulti * val->lineBet / val->times;
				break;
			default:
				multi = (ret->totalMulti - ret->normalMulti) / val->lineNum;
				win = (double)(ret->totalMulti - ret->normalMulti) * val->lineBet / val->times;
				break;
		}

		for(index = 0; index < FR_PAYDISTVALUE_LEN; index++)
		{
			if(multi >= frPayoutDistValue[index])
				frPayoutDistLQRound[type][index]++;
		}
		for(index = 0; index < FR_PAYDISTVALUE_LEN; index++)
		{
			if(frPayoutDistValue[index] > multi)
			{
				frPayoutDistBetweenRound[type][index]++;
				frPayoutDistBetweenWin[type][index] += win;
				break;
			}
		}
	}

	for(type = 0; type < FR_SYMBOL_PAYOUTDIST_TYPE; type++)
	{
		double win;
		int i, j, k, icon, num;

		//将免费游戏和购买免费游戏模块分开
		if((!BUYFREEGAME && type >= FR_SYMBOL_PAYOUTDIST_TYPE_BUY_FREE) ||
			(BUYFREEGAME && type < FR_SYMBOL_PAYOUTDIST_TYPE_BUY_FREE))
			continue;

		//避免普通游戏模块计算免费游戏数据
		if(type % 2 > 0 && ret->prizeType == E_PRIZETYPE_NORMAL)
			continue;

		if(type == 0)
		{
			for(i = 0; i < ret->normalLineNum; i++)
			{
				int singleMulti, totalMulti = 0;
				for (k = 0; k < BOARD_COLUMN; k++)
				{
					singleMulti = ret->normalWildMultis[k];
					singleMulti = singleMulti == 1 ? 0 : singleMulti;
					totalMulti += singleMulti;
				}
				totalMulti = totalMulti == 0 ? 1 : totalMulti;
				icon = ret->normalLineInfos[i][1]-1;
				for(num = 1; num <= BOARD_COLUMN; num++)
				{
					if(gIconMultis[icon][num] == (double)ret->normalLineInfos[i][2]/totalMulti)
						break;
				}
				frSymbolPayoutDistWin[type][icon][num] += (double)gIconMultis[icon][num] * totalMulti * val->lineBet / val->times;
			}
		}
		else
		{
			
			for(i = 0; i < ret->freegameNum; i++)
			{
				int singleMulti, totalMulti = 0;
				for (k = 0; k < BOARD_COLUMN; k++)
				{
					singleMulti = ret->freeWildMultis[i][k];
					singleMulti = singleMulti == 1 ? 0 : singleMulti;
					totalMulti += singleMulti;
				}
				totalMulti = totalMulti == 0 ? 1 : totalMulti;
				for(j = 0; j < ret->freegameLineNum[i]; j++)
				{
					icon = ret->freegameLineInfos[i][j][1]-1;
					for(num = 1; num <= BOARD_COLUMN; num++)
					{
						if(gIconMultis[icon][num] == (double)ret->freegameLineInfos[i][j][2]/totalMulti)
							break;
					}
					frSymbolPayoutDistWin[type][icon][num] += (double)gIconMultis[icon][num] * totalMulti * val->lineBet / val->times;
				}
			}
		}
	}
}

void fr_print()
{
#ifndef CHECK_TOMJERRY
	return;
#endif
	LOGI("\n======猫和老鼠参数======");
	LOGI("一般游戏RTP: %.2f%%", frNormalWin / frTotalBet * 100);
	LOGI("免费游戏RTP: %.2f%%", frFreeWin / frTotalBet * 100);
	LOGI("游戏整体RTP: %.2f%%\n", frTotalWin / frTotalBet * 100);

	LOGI("免费游戏平均赔付倍数: %.2f", frFreeWin / frFreeRound / (frTotalBet / frTotalRound));
	LOGI("免费游戏平均场次:     %.2f", (double)frFreeTotalSpins / frFreeRound);
	LOGI("一般游戏触发频率:     %.2f%%", (double)frNormalWinRound / frTotalRound * 100);
	LOGI("免费游戏触发频率:     %.2f%%", (double)frFreeRound / frTotalRound * 100);

	LOGI("一般游戏最低倍数: %.2f", frNormalMinMulti);
	LOGI("一般游戏最高倍数: %.2f", frNormalMaxMulti);
	LOGI("免费游戏最低倍数: %.2f", frFreeMinMulti);
	LOGI("免费游戏最高倍数: %.2f\n", frFreeMaxMulti);

#ifndef CHECK_TOMJERRY
	int i;
	printf("0个SC\t1个SC\t2个SC\t3个SC\t4个SC\t5个SC\n");
	for (i = 0; i <= BOARD_COLUMN; i++)
	{
		printf("%d\t", gSCatNum[i]);
	}
	printf("\n");

	int type;
	LOGI("[开奖分布]");
	char *titles[FR_PAYOUTDIST_TYPE] = {"游戏整体:", "一般游戏:", "免费游戏:"};
	long totalRound[FR_PAYOUTDIST_TYPE] = {frTotalRound, frTotalRound, frFreeRound};
	for(type = 0; type < FR_PAYOUTDIST_TYPE; type++)
	{
		int i;
		LOGI("%s\n--------------------------------------------------------------------------------------------", titles[type]);
		LOGI("|   赢倍  |   赢分   |  HIT RATE  | 累计机率 |   赢倍区间   | HIT RATE   |   机率  | RTP    |");
		for(i = FR_PAYDISTVALUE_LEN - 1; i >= 0; i--)
		{
			LOGI("|  %5d  |  %6d  | %10.2f | %7.3f%% | %5d-%-6d | %10.2f | %6.2f%% | %5.2f%% |", 
				frPayoutDistValue[i], frPayoutDistValue[i] * BET_LINE_NUM, (frPayoutDistLQRound[type][i] == 0 ? 0 : (double)totalRound[type] / frPayoutDistLQRound[type][i]), (double)frPayoutDistLQRound[type][i] / totalRound[type] * 100,
				(i == 0 ? 0 : i == FR_PAYDISTVALUE_LEN - 1 ? frPayoutDistValue[i] : frPayoutDistValue[i]), i == FR_PAYDISTVALUE_LEN - 1 ? frPayoutDistValue[i]*10 : frPayoutDistValue[i+1], 
				(frPayoutDistBetweenRound[type][i] == 0 ? 0 : (double)totalRound[type] / frPayoutDistBetweenRound[type][i]), (double)frPayoutDistBetweenRound[type][i] / totalRound[type] * 100, frPayoutDistBetweenWin[type][i] / frTotalBet * 100);
		}
		LOGI("--------------------------------------------------------------------------------------------");
	}

	LOGI("\n[图标开奖分布]");
	char *titles2[FR_SYMBOL_PAYOUTDIST_TYPE] = {"一般游戏:", "免费游戏:", "购买免费游戏:"};
	char *symbolStr[ICON_NUM] = {"PIC1", "PIC2", "PIC3", "PIC4", "PIC5", "A", "K", "Q", "J", "10", "WILD", "CAT", "SC"};
	for(type = 0; type < FR_SYMBOL_PAYOUTDIST_TYPE; type++)
	{
		int icon;
		LOGI("%s\n------------------------------------------------------\n| SYMBOL| 5of    | 4of    | 3of    | 2of    | 1of    |", titles2[type]);
		for(icon = 0; icon < ICON_NUM; icon++)
		{
			LOGI("| %5s | %5.2f%% | %5.2f%% | %5.2f%% | %5.2f%% | %5.2f%% |", symbolStr[icon],
				frSymbolPayoutDistWin[type][icon][5] / frTotalBet * 100,
				frSymbolPayoutDistWin[type][icon][4] / frTotalBet * 100,
				frSymbolPayoutDistWin[type][icon][3] / frTotalBet * 100,
				frSymbolPayoutDistWin[type][icon][2] / frTotalBet * 100,
				frSymbolPayoutDistWin[type][icon][1] / frTotalBet * 100);
		}
		LOGI("------------------------------------------------------");
	}
	#endif
}

STR_AlgKillFishResult* protocol_mammonKillFish_2(int curBetLineNum, int wantTableId)
{
	static STR_AlgKillFish val = {0};
	static STR_AlgKillFishResult ret = {0};

	val.times = 1;
	val.lineBet = 10 * val.times;
	val.lineNum = curBetLineNum;
	val.tableId = wantTableId;
	val.blackDiff = 6;
	val.blackDiffType = 5;
	val.uid = 5;
	val.playerDiff = 0;
	if(BUYFREEGAME)
		val.buyFreegameScore = 100 * val.lineBet * val.lineNum;  //购买免费游戏
	else
		val.buyFreegameScore = 0;
	alg_kill_fish(&val, &ret);

	if(BUYFREEGAME)
		gTotalBet[gMachineIndex] += (double)val.buyFreegameScore / val.times;
	else
		gTotalBet[gMachineIndex] += (double)val.lineNum * val.lineBet / val.times;
	gTotalWin[gMachineIndex] += (double)ret.totalMulti * val.lineBet / val.times;

    if (ret.totalMulti != 0)
    {
        gWinRounds[gMachineIndex]++;
        gEmptyRounds[gMachineIndex] = 0;
        if (ret.prizeType == E_PRIZETYPE_FREEGAME)
        {
            if (ret.totalMulti > gMaxMultiFree[gMachineIndex])
                gMaxMultiFree[gMachineIndex] = ret.totalMulti;
            if (ret.totalMulti < gMinMultiFree[gMachineIndex])
                gMinMultiFree[gMachineIndex] = ret.totalMulti;
        }
        else
        {
            if (ret.totalMulti > gMaxMultiNormal[gMachineIndex])
                gMaxMultiNormal[gMachineIndex] = ret.totalMulti;
        }
        if ((gTotalBet[gMachineIndex] - gTotalWin[gMachineIndex]) < gMaxSend[gMachineIndex])
            gMaxSend[gMachineIndex] = gTotalBet[gMachineIndex] - gTotalWin[gMachineIndex];
    }
    else
    {
        gEmptyRounds[gMachineIndex]++;
        if (gEmptyRounds[gMachineIndex] > gMaxEmptyRounds[gMachineIndex])
            gMaxEmptyRounds[gMachineIndex] = gEmptyRounds[gMachineIndex];
    }
    
    if (ret.prizeType == E_PRIZETYPE_FREEGAME)
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
	fr_handle(&val, &ret);

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

    memset(gMinMultiFree, 0, sizeof(gMinMultiFree));
    memset(gMinMultiSpecial, 0, sizeof(gMinMultiSpecial));
	param_clear();
	fr_clear();
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
			protocol_mammonKillFish(BET_LINE_NUM);
			// usleep((int)(0.01 * 1000000));
		}
        LOGI("----------------------------------Machine:%d--------------------------------------", gMachineIndex);
		protocol_mammonGetData(1);
	}

    LOGI("\n难度\t线数\t抽样号\t总局数\t中奖局数\t免费局数\t连续0分局数\t总押\t总赢\t\t免费赢\t\t最大负分\t普通最高倍率\t免费最低倍率\t免费最高倍率");
    for (i = 0; i < MACHINE_NUMS; i++)
    {
        printf("\n%d\t%d\t%d\t%d\t%d\t\t%d\t\t%d\t%.2f\t%.2f\t%.2f\t%.1f\t\t%u\t\t%u\t\t%u",DIFF, BET_LINE_NUM, i, TOTAL_ROUNDS, gWinRounds[i], gFreeRounds[i], 
		gMaxEmptyRounds[i], gTotalBet[i], gTotalWin[i], gFreeWin[i], gMaxSend[i]/COIN_RATE, gMaxMultiNormal[i]/BET_LINE_NUM, gMinMultiFree[i]/BET_LINE_NUM, gMaxMultiFree[i]/BET_LINE_NUM);
    }

	param_print();
	fr_print();

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