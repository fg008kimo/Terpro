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

//dongyuan指标
#define CHECK_DONGYUAN
long dyTotalRound, dyNormalWinRound, dyFreeRound, dyTotalWinNum, dyRespinRound, dyRespinTotalSpins;
double dyTotalBet, dyTotalWin, dyNormalWin, dyFreeWin, dyRespinWin;
double dyNormalMinMulti, dyNormalMaxMulti, dyFreeMinMulti, dyFreeMaxMulti, dyRespinMinMulti, dyRespinMaxMulti;
#define DY_PAYOUTDIST_TYPE 4
enum dy_payoutDist_types {DY_PAYOUTDIST_TYPE_TOTAL, DY_PAYOUTDIST_TYPE_NORMAL, DY_PAYOUTDIST_TYPE_FREE, DY_PAYOUTDIST_TYPE_RESPIN};
#define DY_PAYDISTVALUE_LEN 15
const int dyPayoutDistValue[DY_PAYDISTVALUE_LEN] = {1, 5, 10, 20, 30, 50, 100, 200, 300, 400, 500, 1000, 2000, 5000, 10000};
long dyPayoutDistLQRound[DY_PAYOUTDIST_TYPE][DY_PAYDISTVALUE_LEN];
long dyPayoutDistBetweenRound[DY_PAYOUTDIST_TYPE][DY_PAYDISTVALUE_LEN];
double dyPayoutDistBetweenWin[DY_PAYOUTDIST_TYPE][DY_PAYDISTVALUE_LEN];
#define DY_SYMBOL_PAYOUTDIST_TYPE 3
enum dy_symbol_payoutDist_types {DY_SYMBOL_PAYOUTDIST_TYPE_NORMAL, DY_SYMBOL_PAYOUTDIST_TYPE_FREE, DY_SYMBOL_PAYOUTDIST_TYPE_RESPIN};
double dySymbolPayoutDistWin[DY_SYMBOL_PAYOUTDIST_TYPE];

#ifdef NORMAL_WIN_NUM
int dyNormalWinNumDist[500500501];
//#define NORMAL_WIN_NUM_PRINT	1
#endif

void freeGame_clean()
{
#ifdef CHECK_FREEGAME
	freeGameTotalRound = 0;
	freeGameFreeRound = 0;
	freeGameCurSeqNonRound = 0;
	freeGameMaxSeqNonRound = 0;
	freeGameTotalMulti = 0;
	freeGameFreeMulti = 0;
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
	LOGI("平均倍率：	      	%f", freeGameFreeMulti/freeGameFreeRound);
	LOGI("出分占比：	      	%f%%", freeGameFreeMulti*100/freeGameTotalMulti);
	LOGI("====================");
#endif
}

void respinGame_clean()
{
#ifdef CHECK_RESPINGAME
	respinTotalRound = 0;
	respinFreeRound = 0;
	respinCurSeqNonRound = 0;
	respinMaxSeqNonRound = 0;
	respinTotalMulti = 0;
	respinFreeMulti = 0;
#endif
}

void respinGame_handle(STR_AlgKillFish* val, STR_AlgKillFishResult* ret)
{
#ifdef CHECK_RESPINGAME
	respinTotalRound++;
	respinTotalMulti += ret->totalMulti;
	if(ret->prizeType == E_PRIZETYPE_RESPINGAME)
	{
		respinFreeRound++;
		respinCurSeqNonRound = 0;
		respinFreeMulti += ret->totalMulti;
	}
	else
	{
		respinCurSeqNonRound++;
		if(respinCurSeqNonRound > respinMaxSeqNonRound)
			respinMaxSeqNonRound = respinCurSeqNonRound;
	}
#endif
}

void respinGame_print()
{
#ifdef CHECK_RESPINGAME
	LOGI("\n=====重轉游戏======");
	LOGI("平均几局出一次：      %f", (double)respinTotalRound/respinFreeRound);
	LOGI("最大连续不中局数：    %d", respinMaxSeqNonRound);
	LOGI("平均倍率：	      %f", respinFreeMulti/respinFreeRound);
	LOGI("出分占比：	      %f%%", respinFreeMulti*100/respinTotalMulti);
	LOGI("====================");
#endif
}

void param_clear()
{
	freeGame_clean();
	respinGame_clean();
}

void param_handle(STR_AlgKillFish* val, STR_AlgKillFishResult* ret)
{
	freeGame_handle(val, ret);
	respinGame_handle(val, ret);
}

void param_print()
{
	freeGame_print();
	respinGame_print();
}

void dy_clear()
{
#ifndef CHECK_DONGYUAN
	return;
#endif
	dyTotalRound = dyNormalWinRound = dyFreeRound = dyTotalWinNum = dyRespinRound = 0;
	dyTotalBet = dyTotalWin = dyNormalWin = dyFreeWin = dyRespinWin = 0;
	dyNormalMinMulti = dyFreeMinMulti = dyRespinMinMulti = 10000;
	dyNormalMaxMulti = dyFreeMaxMulti = dyRespinMaxMulti = 0;
	memset(dyPayoutDistLQRound, 0, sizeof(dyPayoutDistLQRound));
	memset(dyPayoutDistBetweenRound, 0, sizeof(dyPayoutDistBetweenRound));
	memset(dyPayoutDistBetweenWin, 0, sizeof(dyPayoutDistBetweenWin));
	memset(dySymbolPayoutDistWin, 0, sizeof(dySymbolPayoutDistWin));
#ifdef NORMAL_WIN_NUM
	memset(dyNormalWinNumDist, 0, sizeof(dyNormalWinNumDist));
#endif
}

void dy_handle(STR_AlgKillFish* val, STR_AlgKillFishResult* ret)
{
#ifndef CHECK_DONGYUAN
	return;
#endif
	dyTotalRound++;
	dyTotalBet += (double)val->lineNum * val->lineBet / val->times;
	dyTotalWin += (double)ret->totalMulti / val->times;//totalMulti裡面的freegameMultis沒有*betScore的話分數會不對,但部屬上服務器他們只要各Multis的盤面原始分數
	//dyTotalWin += (double)(ret->normalMulti + (ret->freegameMultis) + ret->respinMulti) / val->times;
	if(ret->normalMulti > 0)
	{
		int arr_index;
		dyNormalWinRound++;
		dyNormalWin += (double)ret->normalMulti / val->times;
		dyNormalMinMulti = (dyNormalMinMulti > (double)ret->normalMulti / val->lineNum) ? (double)ret->normalMulti / val->lineNum: dyNormalMinMulti;
		dyNormalMaxMulti = ((double)ret->normalMulti / val->lineNum> dyNormalMaxMulti) ? (double)ret->normalMulti / val->lineNum : dyNormalMaxMulti;
		#ifdef NORMAL_WIN_NUM
		if(ret->normalMulti <= 1){
			if(ret->normalMulti == 0.1)
				arr_index = 2;
			if(ret->normalMulti == 0.5)
				arr_index = 3;
		}else
			arr_index = ret->normalMulti;
		dyNormalWinNumDist[arr_index]++;
		#ifdef NORMAL_WIN_NUM_PRINT
		printf("ret->normalMulti:%lf, dyNormalWinNumDist[arr_index]:%d, line:%d in NORMAL_WIN_NUM part\n", ret->normalMulti, dyNormalWinNumDist[arr_index], __LINE__);
		#endif
		#endif
	}
	if(ret->prizeType == E_PRIZETYPE_FREEGAME)
	{
		s64 freeMulti = ret->freegameMultis;	
		dyFreeRound++;
		dyFreeWin += (double)freeMulti / val->times;
		dyFreeMinMulti = (dyFreeMinMulti > (double)freeMulti / val->lineNum) ? (double)freeMulti / val->lineNum : dyFreeMinMulti;
		dyFreeMaxMulti = ((double)freeMulti / val->lineNum > dyFreeMaxMulti) ? (double)freeMulti / val->lineNum : dyFreeMaxMulti;		
	}
	if(ret->prizeType == E_PRIZETYPE_RESPINGAME)
	{
		s64 respinMulti = ret->respinMulti;	
		dyRespinRound++;
		dyRespinWin += (double)respinMulti / val->times;
		dyRespinMinMulti = (dyRespinMinMulti > (double)respinMulti) ? (double)respinMulti / val->lineNum : dyRespinMinMulti;
		dyRespinMaxMulti = ((double)respinMulti / val->lineNum > dyRespinMaxMulti) ? (double)respinMulti / val->lineNum : dyRespinMaxMulti;		
	}
	
	if(ret->totalMulti > 0)
		dyTotalWinNum++;
	
	int type;
	for(type = 0; type < DY_PAYOUTDIST_TYPE; type++) //關掉是因為跑一億有時候程式會segmention fault <= 結果根本不是  關掉還是會segmention fault
	{
		int multi, index;
		double win;
	
		if(//(type == DY_PAYOUTDIST_TYPE_FREE && ret->prizeType == E_PRIZETYPE_NORMAL) ||
		   (type == DY_PAYOUTDIST_TYPE_FREE && ret->prizeType == E_PRIZETYPE_RESPINGAME) ||
		   //(type == DY_PAYOUTDIST_TYPE_NORMAL && ret->prizeType == E_PRIZETYPE_FREEGAME) ||
		   //(type == DY_PAYOUTDIST_TYPE_NORMAL && ret->prizeType == E_PRIZETYPE_RESPINGAME) ||
		   //(type == DY_PAYOUTDIST_TYPE_RESPIN && ret->prizeType == E_PRIZETYPE_NORMAL) ||
		   (type == DY_PAYOUTDIST_TYPE_RESPIN && ret->prizeType == E_PRIZETYPE_FREEGAME))
			continue;
	
		switch(type)
		{
			case DY_PAYOUTDIST_TYPE_TOTAL:
				multi = ret->totalMulti;
				//multi = (ret->normalMulti + (ret->freegameMultis * val->betScore) + ret->respinMulti);
				win = (double)ret->totalMulti / val->times;// * val->betScore
				//win = (double)(ret->normalMulti + (ret->freegameMultis * val->betScore) + ret->respinMulti) / val->times;// * val->betScore
				break;
			case DY_PAYOUTDIST_TYPE_NORMAL:
				multi = ret->normalMulti;
				win = (double)ret->normalMulti / val->times;// * val->betScore
				break;
			//case DY_PAYOUTDIST_TYPE_FREE://rotary game
			//	multi = ret->freegameMultis;
			//	win = (double)ret->freegameMultis / val->times;// * val->betScore 
			//	break;
			//case DY_PAYOUTDIST_TYPE_RESPIN://respin game
			//	multi = ret->respinMulti;
			//	win = (double)ret->respinMulti/ val->times;// * val->betScore / val->times;
			//	break;			
			//default://all free game
			//	multi = ret->totalMulti - ret->normalMulti;
			//	win = (double)(ret->totalMulti - ret->normalMulti) / val->times;// * val->betScore / val->times;
			//	break;
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
		//if(index == DY_PAYDISTVALUE_LEN)
		//{
		//	LOGE("dyPayoutDistBetweenRound error: multi=%d\n", multi);
		//}
	}

	for(type = 0; type < DY_SYMBOL_PAYOUTDIST_TYPE; type++)
	{
		double win;

		#ifdef DEBUG_PAYOUT_DIST
		printf("type:%d, ret->prizeType:%d, line:%d in dy_handle()\n", type, ret->prizeType, __LINE__);
		#endif

		if(((type == 1) && ret->prizeType == E_PRIZETYPE_NORMAL) || 
		   ((type == 2) && ret->prizeType == E_PRIZETYPE_FREEGAME) ||
		   ((type == 2) && ret->prizeType == E_PRIZETYPE_NORMAL) ||
		   ((type == 1) && ret->prizeType == E_PRIZETYPE_RESPINGAME))
			continue;

		if(type == 0)
		{
			dySymbolPayoutDistWin[type] += (double)ret->normalMulti / val->times;
			#ifdef DEBUG_PAYOUT_DIST
			printf("type:%d, dySymbolPayoutDistWin[type]:%lf, line:%d in dy_handle()\n", type, dySymbolPayoutDistWin[type], __LINE__);
			#endif
		}
		else if(type == 1)
		{
			if(ret->prizeType == E_PRIZETYPE_FREEGAME){
				dySymbolPayoutDistWin[type] += (double)ret->freegameMultis / val->times;
				#ifdef DEBUG_PAYOUT_DIST
				printf("type:%d, dySymbolPayoutDistWin[type]:%lf, line:%d in dy_handle()\n", type, dySymbolPayoutDistWin[type], __LINE__);
				#endif
			}
		}
		else if(type == 2)
		{
			if(ret->prizeType == E_PRIZETYPE_RESPINGAME){
				dySymbolPayoutDistWin[type] += (double)ret->respinMulti / val->times;
				#ifdef DEBUG_PAYOUT_DIST
				printf("type:%d, dySymbolPayoutDistWin[type]:%lf, line:%d in dy_handle()\n", type, dySymbolPayoutDistWin[type], __LINE__);
				#endif
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
	LOGI("輪盤游戏RTP: %.4f%%", dyFreeWin / dyTotalBet * 100);
	LOGI("重轉游戏RTP: %.4f%%", dyRespinWin / dyTotalBet * 100);
	LOGI("游戏整体RTP: %.4f%%\n", dyTotalWin / dyTotalBet * 100);

	LOGI("輪盤游戏平均赔付倍数: %.4f", dyFreeWin / dyFreeRound / (dyTotalBet / dyTotalRound));
	LOGI("一般游戏贏錢率: %.4f%%", (double)dyNormalWinRound / dyTotalRound * 100);
	LOGI("輪盤游戏觸發率: %.4f%%", (double)dyFreeRound / dyTotalRound * 100);
	LOGI("重轉游戏觸發率: %.4f%%", (double)dyRespinRound / dyTotalRound * 100);
	LOGI("整體游戏贏錢率: %.4f%%\n", (double)dyTotalWinNum / dyTotalRound * 100);

	LOGI("一般游戏最低倍数: %.4f", dyNormalMinMulti);
	LOGI("一般游戏最高倍数: %.4f", dyNormalMaxMulti);
	LOGI("輪盤游戏最低倍数: %.4f", dyFreeMinMulti);
	LOGI("輪盤游戏最高倍数: %.4f\n", dyFreeMaxMulti);
	LOGI("重轉游戏最低倍数: %.4f", dyRespinMinMulti);
	LOGI("重轉游戏最高倍数: %.4f\n", dyRespinMaxMulti);	

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
	//			dyPayoutDistValue[i], dyPayoutDistValue[i], (dyPayoutDistLQRound[type][i] == 0 ? 0 : (double)totalRound[type] / dyPayoutDistLQRound[type][i]), (double)dyPayoutDistLQRound[type][i] / totalRound[type] * 100,
	//			(i == 0 ? 0 : dyPayoutDistValue[i-1]), dyPayoutDistValue[i], (dyPayoutDistBetweenRound[type][i] == 0 ? 0 : (double)totalRound[type] / dyPayoutDistBetweenRound[type][i]), (double)dyPayoutDistBetweenRound[type][i] / totalRound[type] * 100, dyPayoutDistBetweenWin[type][i] / dyTotalBet * 100);
	//	}
	//	LOGI("---------------------------------------------------------------------------------");
	//}

	LOGI("\n[开奖分布]");
	char *titles2[DY_SYMBOL_PAYOUTDIST_TYPE] = {"一般游戏:", "輪盤游戏:", "重轉游戏:"};
	for(type = 0; type < DY_SYMBOL_PAYOUTDIST_TYPE; type++){
		int icon;
		LOGI("%s\n------------------------------------------------------\n|  value  |", titles2[type]);
		LOGI("| %5.4f%% |", dySymbolPayoutDistWin[type] / dyTotalBet * 100);
		LOGI("------------------------------------------------------");
	}
	#ifdef NORMAL_WIN_NUM
	LOGI("\n[普通遊戲場數分布]");
	int i;
	double arr;
	LOGI("------------------------------------------------------\n| normal score|     number|");
	for(i = 0; i < 500500501; i ++){
		if(dyNormalWinNumDist[i] > 0){
			if(i == 2)
				arr = 0.1;
			else if(i == 3)
				arr = 0.5;
			else
				arr = (double)i;
			LOGI("| %11lf | %10d|", arr, dyNormalWinNumDist[i]);
			#ifdef NORMAL_WIN_NUM_PRINT
			printf("\n dyNormalWinNumDist[%d]:%d\n", i, dyNormalWinNumDist[i]);
			#endif
		}
	}
	LOGI("------------------------------------------------------");
	#endif	
	
	LOGI("====================");
}

STR_AlgKillFishResult* protocol_mammonKillFish_2(int wantTableId)//wantTableId = 1
{
	static STR_AlgKillFish val = {0};
	static STR_AlgKillFishResult ret = {0};

	val.set = 4;//1:USD  2:MYR  3:MMK  4:IDR
	val.times = 1000000;//讓betScore維持整數(這款遊戲最小單位 = 0.1,但times定100以防以後有0.01)。server times = 1000000(本地算rtp, times要用100,不然會算不出正確數值。這裡的val.times = Alg.c的board_times)
	val.lineBet = 1 * val.times;
	val.lineNum = 500;//1:1 5 10 50 100(lineBet:100000)    2:1 5 10 50 100  3:10 50 100 500 1000  4:100 500 1000 5000 10000
	val.tableId = 1;//wantTableId;//1
	val.blackDiff = 4;
	val.blackDiffType = 5;
	val.uid = 0;
	val.playerDiff = 4;	
	alg_kill_fish(&val, &ret);

	gTotalBet[gMachineIndex] += (double)val.lineNum * val.lineBet / val.times;
	gTotalWin[gMachineIndex] += (double)ret.totalMulti / val.times;
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
        gFreeWin[gMachineIndex] += (double)ret.totalMulti / val.times;
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
			if(TOTAL_ROUNDS < 1000000)
				printf("-*-*-*-*-*-*-*-test round:%d in test_main_real()\n",i);
			if((TOTAL_ROUNDS >= 1000000) && (i % 1000000 == 0)){
				printf("-*-*-*-*-*-*-*-test round:%d in test_main_real()\n",i);
				snprintf(buf, sizeof(buf), "date");
				system(buf);
			}
			protocol_mammonKillFish();
			// usleep((int)(0.01 * 1000000));
		}
        LOGI("----------------------------------Machine:%d--------------------------------------", gMachineIndex);
		protocol_mammonGetData(1);
	}

    for (i = 0; i < MACHINE_NUMS; i++)
    {
        LOGI("\n难度\t抽样号\t总局数\t\t中奖局数\t免费局数(重轉+輪盤)\t连续0分局数\t总押\t\t总赢\t\t免费赢\t\t最大负分\t\t普通最高倍率\t免费最低倍率\t免费最高倍率");
		printf("\n%d\t%d\t%d\t\t%d\t\t%d\t\t%d\t\t%.2f\t%.2f\t%.2f\t%.1f\t\t\t%u\t\t%u\t\t%u",DIFF, i, TOTAL_ROUNDS, gWinRounds[i], gFreeRounds[i], gMaxEmptyRounds[i], gTotalBet[i], gTotalWin[i], gFreeWin[i], gMaxSend[i]/COIN_RATE, gMaxMultiNormal[i], gMinMultiFree[i], gMaxMultiFree[i]);
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