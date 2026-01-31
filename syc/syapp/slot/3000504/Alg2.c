#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

#include "server.h"
#include "Alg2.h"
#include "data.h"
#include "logic/board.h"
#include "logic/freegame.h"

//#define ALG_CODE_FLOW					1
// #define _BIG_PRIZE_TEST_

STR_AlgData gAlgData[MAX_TABLES_NUM];

int port;

//缓存STR_AlgKillFishResult信息，以节省开辟内存时间 //缓存=cache
STR_AlgKillFishResultTemp gResult;

int is_dajiang_mode()
{
#ifdef _BIG_PRIZE_TEST_
	return 1;
#else
	return 0;
#endif
}

int get_alg_ver(void)
{
if (!is_dajiang_mode())
	return ALG_VERSION;
else
	return ALG_VERSION_BIG_PRIZE;	
}

void set_port(int new_port)
{
	port = new_port;
}

int get_diff(s32 tableId)
{
	int diff;

	diff = gAlgData[tableId].diff;

	return diff % MAX_DIFF_NUM;
}

void table_in(s32 tableId, double betScore)
{
	double betCoins;

	betCoins = betScore / gAlgData[tableId].coin_rate;
	gAlgData[tableId].gRealInOutCoins[0] += betCoins;
}

void table_out(s32 tableId, double winScore)
{
	double winCoins;

	winCoins = winScore / gAlgData[tableId].coin_rate;
	gAlgData[tableId].gRealInOutCoins[1] += winCoins;
}

void table_clear(s32 tableId)
{
	gAlgData[tableId].gRealInOutCoins[0] = 0;
	gAlgData[tableId].gRealInOutCoins[1] = 0;
}

int get_totalWinMulti(s8 prizeType, u32 normalMulti, int *freegameMultis, s16 freegameNum)
{
	int totalMulti;
	int i;

	totalMulti = normalMulti;  
	#ifdef DEBUG_PAYOUT_DIST
	printf("totalMulti:%d, %s\n", totalMulti, __FUNCTION__);
	#endif
	if(prizeType == E_PRIZETYPE_FREEGAME)
	{
		for(i = 0; i < freegameNum; i++){
			totalMulti += freegameMultis[i];
			#ifdef DEBUG_PAYOUT_DIST
			printf("totalMulti:%d, freegameMultis[i]:%d, %s\n", totalMulti, freegameMultis[i], __FUNCTION__);
			#endif
		}
	}

	return totalMulti;
}

void cpy_result(STR_AlgKillFishResult* target, STR_AlgKillFishResultTemp* source)
{
	target->prizeType = source->prizeType;
	target->totalMulti = source->totalMulti;

	target->normalIcons = source->normalIcons;
	target->normalMulti = source->normalMulti;
	target->normalLineInfos = source->normalLineInfos;
	//target->multiplier = source->multiplier;
	#ifdef REEL_DIST
	target->rollerDist = source->rollerDist;
	target->rollerDist1 = source->rollerDist1;
	target->rollerDist2 = source->rollerDist2;
	target->rollerDist3 = source->rollerDist3;
	target->rollerDist4 = source->rollerDist4;
	target->rollerDist5 = source->rollerDist5;
	target->rollerDist6 = source->rollerDist6;
	target->rollerDist7 = source->rollerDist7;
	target->rollerDist8 = source->rollerDist8;
	#endif
	#ifdef GET_RAND
	target->rand_num = source->rand_num;
	#endif
	#ifdef SRAND
	target->rand_num1 = source->rand_num1;
	#endif
	#ifdef WILD_MUL_DIST
	target->wildMul = source->wildMul;
	#endif

	if(source->prizeType == E_PRIZETYPE_FREEGAME)
	{
		//target->freegameIconsBefore = source->freegameIconsBefore;
		target->freegameIcons = source->freegameIcons;
		target->freegameMultis = source->freegameMultis;
		target->freegameLineInfos = source->freegameLineInfos;
		target->freegameLineNum = source->freegameLineNum;		
		#ifdef FG_WILD_NUM
		target->freegameWildNum = source->freegameWildNum;
		#endif
		#ifdef FG_WILD_FINAL_NUM
		target->freegameWildFinalNum = source->freegameWildFinalNum;
		#endif
		#ifdef FG_ADD_NUM
		target->freegameAddNum = source->freegameAddNum;
		#endif
		#ifdef FG_SC_DIST
		target->freegameSCcal = source->freegameSCcal;
		#endif
		#ifdef FG_SC_NUM
		target->freegameSCNum = source->freegameSCNum;
		#endif
		//target->freegameMultiplier = source->freegameMultiplier;
		//int k;
		//for(k = 0; k < target->freegameNum; k++){
		//	//for(i = 0; i < ret->freegameLineNum[k]; i++)
		//	printf("k:%d, target->freegameLineNum[k]:%d, source->freegameLineNum[k]:%d, line:%d\n", k, target->freegameLineNum[k], source->freegameLineNum[k], __LINE__);
		//}
		target->scArray = source->scArray;
	}
	target->normalLineNum = source->normalLineNum;
	target->freegameNum = source->freegameNum;
}

#ifdef SELF_ARRAY_TEST
void test_function(STR_AlgKillFishResultTemp* ret)
{
	int i, j, k;
	printf("==================self test function start==================\n");
	printf("parameter 1\n");
	for(i = 0; i < BOARD_ICON_NUM; i++){
		printf("normalIcons[%d]:%d\n", i, ret->normalIcons[i]);
	}
	printf("parameter 2\n");
	if(ret->normalLineNum > 0){
		for(i = 0; i < ret->normalLineNum; i++){
			for(j = 0; j < 4; j++)
				printf("normalLineInfos[%d][%d]:%d\n", i, j, ret->normalLineInfos[i][j]);
		}
	}
	printf("parameter 3\n");
	printf("normalMulti:%d\n", ret->normalMulti);
	printf("parameter 4\n");
	printf("prizeType:%d\n", ret->prizeType);
	if(ret->prizeType == E_PRIZETYPE_FREEGAME){
		for(k = 0; k < ret->freegameNum; k++){
			printf("free game start==================\n");
			printf("parameter 5 at freegame:%d\n", k);
			printf("freegameNum:%d\n", ret->freegameNum);
			printf("parameter 6 at freegame:%d\n", k);
			for(i = 0; i < BOARD_ICON_NUM; i++){
				printf("freegameIcons[%d][%d]:%d\n", k, i, ret->freegameIcons[k][i]);
			}
			printf("parameter 7 at freegame:%d\n", k);
			if(ret->freegameLineNum[k] > 0){
				for(i = 0; i < ret->freegameLineNum[k]; i++){
					for(j = 0; j < 4; j++)
						printf("freegameLineInfos[%d][%d][%d]:%d, ret->freegameLineNum[k]:%d\n", k, i, j, ret->freegameLineInfos[k][i][j], ret->freegameLineNum[k]);
				}
			}
			printf("parameter 8 at freegame:%d\n", k);
			for(i = 0; i < BOARD_ICON_NUM; i++){
				printf("scArray[%d][%d]:%d\n", k, i, ret->scArray[k][i]);
			}
			printf("parameter 9 at freegame:%d\n", k);
			printf("freegameMultis[k]:%d\n", ret->freegameMultis[k]);
			printf("free game end==================\n");
		}
	}
	printf("parameter 10\n");
	printf("totalMulti:%d\n", ret->totalMulti);
	printf("==================self test function end==================\n");
}
#endif

void alg_kill_fish(STR_AlgKillFish* val, STR_AlgKillFishResult* ret)
{
	s32 tableId;
	double singleLineBet;
	s8 isBuyFreegame;
	double totalBet, totalWin;

	if (val->tableId < 0 || val->tableId >= MAX_TABLES_NUM)// 
	{
		val->tableId = DEFAULT_TABLE_ID;//DEFAULT_TABLE_ID = 0
		//LOGE("function: %s, Line: %d, val->tableId error: %d\n", __FUNCTION__, __LINE__, val->tableId);
	}

	if (val->times <= 0)
	{
		LOGE("function: %s, Line: %d, val->times error: %d\n", __FUNCTION__, __LINE__, val->times);
	}

	if (val->lineBet <= 0)
	{
		LOGE("function: %s, Line: %d, val->lineBet error: %ld\n", __FUNCTION__, __LINE__, val->lineBet);
	}

	if (val->lineNum < 0 || val->lineNum > BET_LINE_TOTAL)
	{
		LOGE("function: %s, Line: %d, val->lineNum error: %hd\n", __FUNCTION__, __LINE__, val->lineNum);
	}

	tableId = val->tableId;
	singleLineBet = (double)val->lineBet / val->times;
	isBuyFreegame = (val->buyFreegameScore > 0);
	
	//test
	#ifdef GET_RAND
	cal_rand(&gResult.rand_num);
	#endif
	#ifdef SRAND
	static int isFirst = 1;
	if(isFirst)
	{
		isFirst = 0;
		srand(time(NULL));
	}
	cal_rand_inbuild(&gResult.rand_num1);
	#endif

	//cal in
	if(isBuyFreegame)
		totalBet = (double)val->buyFreegameScore / val->times;
	else
		totalBet = singleLineBet * val->lineNum;
	table_in(tableId, totalBet);

	#ifdef TEST_TRANVERSAL   //test tranversal reel
	if(is_dajiang_mode()) //大奖模式   
	{
		if(isBuyFreegame || GetRandom() >= 0.5)
			board_traverse_gen(gBuyFreeGameRollerIcon, gBuyFreeGameRollerIconNum, gResult.normalIcons);
		else
			board_traverse_gen(gRollerIcon, gRollerIconNum, gResult.normalIcons);
	}
	else
	{
		if(isBuyFreegame)
			board_traverse_gen(gBuyFreeGameRollerIcon, gBuyFreeGameRollerIconNum, gResult.normalIcons);
		else
			board_traverse_gen(gRollerIcon, gRollerIconNum, gResult.normalIcons);
	}
	//printf("test tranversal in alg_kill_fish()\n");
	#else
	//logic
	if(is_dajiang_mode()) //大奖模式
	{
		if(isBuyFreegame || GetRandom() >= 0.5)
			#ifdef REEL_DIST
			board_gen_dist(gBuyBaseGameRollerIcon, gBuyBaseGameRollerIconNum, gResult.normalIcons, gResult.rollerDist, &gResult.rollerDist1, &gResult.rollerDist2, &gResult.rollerDist3, &gResult.rollerDist4, &gResult.rollerDist5, &gResult.rollerDist6, &gResult.rollerDist7, &gResult.rollerDist8);
			#else
			board_gen(gBuyBaseGameRollerIcon, gBuyBaseGameRollerIconNum, gResult.normalIcons);
			#endif
		else
			#ifdef REEL_DIST
			board_gen_dist(gRollerIcon, gRollerIconNum, gResult.normalIcons, gResult.rollerDist, &gResult.rollerDist1, &gResult.rollerDist2, &gResult.rollerDist3, &gResult.rollerDist4, &gResult.rollerDist5, &gResult.rollerDist6, &gResult.rollerDist7, &gResult.rollerDist8);
			#else
			board_gen(gRollerIcon, gRollerIconNum, gResult.normalIcons);
			#endif
	}
	else
	{
		if(isBuyFreegame)
			#ifdef REEL_DIST
			board_gen_dist(gBuyBaseGameRollerIcon, gBuyBaseGameRollerIconNum, gResult.normalIcons, gResult.rollerDist, &gResult.rollerDist1, &gResult.rollerDist2, &gResult.rollerDist3, &gResult.rollerDist4, &gResult.rollerDist5, &gResult.rollerDist6, &gResult.rollerDist7, &gResult.rollerDist8);
			#else
			board_gen(gBuyBaseGameRollerIcon, gBuyBaseGameRollerIconNum, gResult.normalIcons);
			#endif
		else
			#ifdef REEL_DIST
			board_gen_dist(gRollerIcon, gRollerIconNum, gResult.normalIcons, gResult.rollerDist, &gResult.rollerDist1, &gResult.rollerDist2, &gResult.rollerDist3, &gResult.rollerDist4, &gResult.rollerDist5, &gResult.rollerDist6, &gResult.rollerDist7, &gResult.rollerDist8);
			#else
			board_gen(gRollerIcon, gRollerIconNum, gResult.normalIcons);
			#endif
	}
	#endif
	
	#ifdef WILD_MUL_DIST
	wildGetMultiplier(gResult.normalIcons, gBaseGameWeight, MUL_WEIGHT_SUM, gResult.wildMul);
	#else
	wildGetMultiplier(gResult.normalIcons, gBaseGameWeight, MUL_WEIGHT_SUM);
	#endif
	gResult.normalMulti = board_calMulti(val->lineNum, gResult.normalIcons, gResult.normalLineInfos, &gResult.normalLineNum);
	gResult.prizeType = board_calPrizeType(gResult.normalIcons);
	if(isBuyFreegame && gResult.prizeType != E_PRIZETYPE_FREEGAME)
		LOGE("buyFreegame error: prizeType=%d", gResult.prizeType);
	#ifdef DEBUG_PAYOUT_DIST
	printf("gResult.normalMulti:%d, line:%d in alg_kill_fish()\n", gResult.normalMulti, __LINE__);
	#endif
	if(gResult.prizeType == E_PRIZETYPE_FREEGAME)
	{
		int round;
		#ifdef FREEGAME_FLOW
		printf("free game start, line:%d in alg_kill_fish()\n", __LINE__);
		#endif
		#ifdef FG_TEST
		freeGame_gen(isBuyFreegame, gResult.freegameIcons, gResult.scArray, &gResult.freegameNum, gResult.freegameSCcal, gResult.freegameSCNum, gResult.freegameAddNum, gResult.freegameWildNum, gResult.freegameWildFinalNum);//, gResult.freegameAddRound
		#else
		freeGame_gen(isBuyFreegame, gResult.freegameIcons, gResult.scArray, &gResult.freegameNum);
		#endif
		for(round = 0; round < gResult.freegameNum; round++){
			gResult.freegameMultis[round] = board_calMulti(val->lineNum, gResult.freegameIcons[round], gResult.freegameLineInfos[round], &gResult.freegameLineNum[round]);
			#ifdef DEBUG_PAYOUT_DIST
			printf("round:%d, gResult.freegameNum:%d, gResult.freegameMultis[round]:%d, gResult.freegameLineNum[round]:%d, line:%d in alg_kill_fish()\n", round, gResult.freegameNum, gResult.freegameMultis[round], gResult.freegameLineNum[round], __LINE__); 
			#endif
			#ifdef SELF_ARRAY_TEST
			int i;
			for (i = 0; i < BOARD_ICON_NUM; i++){
				printf("round:%d, i:%d, gResult.scArray[round][i]:%d, line:%d in alg_kill_fish()\n", round, i, gResult.scArray[round][i], __LINE__);
			}
			for (i = 0; i < BOARD_ICON_NUM; i++){
				printf("round:%d, i:%d, gResult.freegameIcons[round][i]:%d, line:%d in alg_kill_fish()\n", round, i, gResult.freegameIcons[round][i], __LINE__);
			}
			#endif
		}
		//#ifdef FG_NUM
		//printf("gResult.freegameNum:%d, line:%d in alg_kill_fish()\n", gResult.freegameNum, __LINE__);
		//#endif
	}

	//maxSend
	gResult.totalMulti = get_totalWinMulti(gResult.prizeType, gResult.normalMulti, gResult.freegameMultis, gResult.freegameNum);
	totalWin = singleLineBet * gResult.totalMulti;
	#ifdef DEBUG_PAYOUT_DIST
	printf("gResult.totalMulti:%d , totalWin:%lf line:%d in alg_kill_fish()\n", gResult.totalMulti, totalWin, __LINE__);
	#endif
	
	#ifdef SELF_ARRAY_TEST
	test_function(&gResult);
	#endif

	//cal out
	table_out(tableId, totalWin);
	#ifdef ALG_CODE_FLOW
	printf("table_out %d in alg_kill_fish()\n", __LINE__);
	#endif

	//result
	cpy_result(ret, &gResult);
	#ifdef ALG_CODE_FLOW
	printf("cpy_result %d in alg_kill_fish()\n", __LINE__);
	#endif
}

void alg_init(STR_AlgInit* val, STR_AlgInitResult* ret)
{
    static uint8_t isCheckMultiProb = 0;
	s32 tableId;

	if (val->tableId < 0 || val->tableId >= MAX_TABLES_NUM)
	{
		val->tableId = DEFAULT_TABLE_ID;
		//LOGE("function: %s, Line: %d, val->tableId error: %d\n", __FUNCTION__, __LINE__, val->tableId);
	}

	tableId = val->tableId;
	gAlgData[tableId].coin_rate = 100;
	gAlgData[tableId].diff = 5;
	gAlgData[tableId].max_send = 1000;

	ret->result = 0;
}

void alg_clear_data(STR_ClearAllAlgData* val, STR_ClearAllAlgDataResult* ret)
{
	s32 tableId;
	s32 i;

	ret->result = 1;

	if (val->begin_table_id < 0 || val->begin_table_id >= MAX_TABLES_NUM)
	{
		val->begin_table_id = DEFAULT_TABLE_ID;
		/*LOGE("function: %s, Line: %d, val->begin_table_id error: %d\n", __FUNCTION__, __LINE__, val->begin_table_id);
		ret->result = 0;*/
	}

	if (val->end_table_id < 0 || val->end_table_id >= MAX_TABLES_NUM)
	{
		val->end_table_id = DEFAULT_TABLE_ID;
		/*LOGE("function: %s, Line: %d, val->end_table_id error: %d\n", __FUNCTION__, __LINE__, val->end_table_id);
		ret->result = 0;*/
	}

	if (val->begin_table_id > val->end_table_id)
	{
		LOGE("function: %s, Line: %d, val->begin_table_id(%d) < val->end_table_id(%d) error\n", __FUNCTION__, __LINE__, val->begin_table_id, val->end_table_id);
		ret->result = 0;
	}

	if (ret->result == 0)
	{
		return;
	}

	LOGI("alg_clear_data()\n");

	for (i = val->begin_table_id; i <= val->end_table_id; i++)
	{
		table_clear(i);
	}

	ret->result = 1;
}

void alg_get_alg_data(STR_AlgTestData* val, STR_AlgTestDataResult* ret)
{
	s32 tableId;
	int i;

	if (val->tableId < 0 || val->tableId >= MAX_TABLES_NUM)
	{
		val->tableId = DEFAULT_TABLE_ID;
		//LOGE("function: %s, Line: %d, val->tableId error: %d\n", __FUNCTION__, __LINE__, val->tableId);
	}

	tableId = val->tableId;

	ret->coin_rate = gAlgData[tableId].coin_rate;
	ret->max_send = gAlgData[tableId].max_send;
	ret->real_diff = get_diff(tableId);
	ret->random = get_alg_ver();

	ret->gRejustIn = convert_double_2_ulong(gAlgData[tableId].gRealInOutCoins[0] * gAlgData[tableId].coin_rate);
	ret->gRejustOut = convert_double_2_ulong(gAlgData[tableId].gRealInOutCoins[1] * gAlgData[tableId].coin_rate);
}


void alg_set_max_send(STR_AlgSetMaxSend* val, STR_AlgSetMaxSendResult* ret)
{
	s32 tableId;
	s32 i;

	ret->result = 1;

	if (val->level < 0 || val->level > MAX_SEND_MAX_VALUE)//MAX_SEND_MAX_VALUE = 實驗局數
	{
		LOGE("function: %s, Line: %d, val->level error: %d\n", __FUNCTION__, __LINE__, val->level);
		ret->result = 0;
	}

    if (val->begin_table_id < 0 || val->begin_table_id >= MAX_TABLES_NUM)
	{
		val->begin_table_id = DEFAULT_TABLE_ID;
		/*LOGE("function: %s, Line: %d, val->begin_table_id error: %d\n", __FUNCTION__, __LINE__, val->begin_table_id);
		ret->result = 0;*/
	}

	if (val->end_table_id < 0 || val->end_table_id >= MAX_TABLES_NUM)
	{
		val->end_table_id = DEFAULT_TABLE_ID;
		/*LOGE("function: %s, Line: %d, val->end_table_id error: %d\n", __FUNCTION__, __LINE__, val->end_table_id);
		ret->result = 0;*/
	}

	if (val->begin_table_id > val->end_table_id)
	{
		LOGE("function: %s, Line: %d, val->begin_table_id(%d) < val->end_table_id(%d) error\n", __FUNCTION__, __LINE__, val->begin_table_id, val->end_table_id);
		ret->result = 0;
	}

	if (ret->result == 0)
	{
		return;
	}

	LOGI("alg_set_max_send() level=%d begin_table_id=%d end_table_id=%d\n",
		val->level, val->begin_table_id, val->end_table_id);

	for (i = val->begin_table_id; i <= val->end_table_id; i++)
	{
		tableId = i;
		gAlgData[tableId].max_send = val->level;
		ret->result = gAlgData[tableId].max_send;
		table_clear(tableId);
	}
}


void alg_reset(STR_AlgReset* val, STR_AlgResetResult* ret)
{
	s32 i;

	ret->result = 1;

	if (val->begin_table_id < 0 || val->begin_table_id >= MAX_TABLES_NUM)
	{
		val->begin_table_id = DEFAULT_TABLE_ID;
		/*LOGE("function: %s, Line: %d, val->begin_table_id error: %d\n", __FUNCTION__, __LINE__, val->begin_table_id);
		ret->result = 0;*/
	}

	if (val->end_table_id < 0 || val->end_table_id >= MAX_TABLES_NUM)
	{
		val->end_table_id = DEFAULT_TABLE_ID;
		/*LOGE("function: %s, Line: %d, val->end_table_id error: %d\n", __FUNCTION__, __LINE__, val->end_table_id);
		ret->result = 0;*/
	}

	if (val->begin_table_id > val->end_table_id)
	{
		LOGE("function: %s, Line: %d, val->begin_table_id(%d) < val->end_table_id(%d) error\n", __FUNCTION__, __LINE__, val->begin_table_id, val->end_table_id);
		ret->result = 0;
	}

	if (ret->result == 0)
	{
		return;
	}

	LOGI("alg_reset() coin_rate=%d begin_table_id=%d end_table_id=%d\n",
		val->coin_rate, val->begin_table_id, val->end_table_id);

	for (i = val->begin_table_id; i <= val->end_table_id; i++)
	{
		gAlgData[i].coin_rate = val->coin_rate;
		table_clear(i);
	}

	ret->result = 1;
}


void alg_in_out_revise(STR_AlgInOutRevise* val, STR_AlgInOutReviseResult* ret)
{
	s32 tableId;

	if (val->tableId < 0 || val->tableId >= MAX_TABLES_NUM)
	{
		val->tableId = DEFAULT_TABLE_ID;
		//LOGE("function: %s, Line: %d, val->tableId error: %d\n", __FUNCTION__, __LINE__, val->tableId);
	}

	if (val->times <= 0)
	{
		LOGE("function: %s, Line: %d, val->times error: %d\n", __FUNCTION__, __LINE__, val->times);
	}

	tableId = val->tableId;

	if (val->times <= 0)
	{
		ret->reviseResult = 0;
		return;
	}

	LOGI("Revise in:%ld out:%ld times:%d", val->inScore, val->outScore, val->times);

	ret->reviseResult = 1;
}

void alg_set_diff(STR_AlgSetDiff* val, STR_AlgSetDiffResult* ret)
{
	s32 tableId;
	s32 i;

	ret->result = 1;

	if (val->diff < 0 || val->diff >= MAX_DIFF_NUM)
	{
		LOGE("function: %s, Line: %d, val->diff error: %d\n", __FUNCTION__, __LINE__, val->diff);
		ret->result = 0;
	}

	if (val->begin_table_id < 0 || val->begin_table_id >= MAX_TABLES_NUM)
	{
		val->begin_table_id = DEFAULT_TABLE_ID;
		/*LOGE("function: %s, Line: %d, val->begin_table_id error: %d\n", __FUNCTION__, __LINE__, val->begin_table_id);
		ret->result = 0;*/
	}

	if (val->end_table_id < 0 || val->end_table_id >= MAX_TABLES_NUM)
	{
		val->end_table_id = DEFAULT_TABLE_ID;
		/*LOGE("function: %s, Line: %d, val->end_table_id error: %d\n", __FUNCTION__, __LINE__, val->end_table_id);
		ret->result = 0;*/
	}

	if (val->begin_table_id > val->end_table_id)
	{
		LOGE("function: %s, Line: %d, val->begin_table_id(%d) < val->end_table_id(%d) error\n", __FUNCTION__, __LINE__, val->begin_table_id, val->end_table_id);
		ret->result = 0;
	}

	if (ret->result == 0)
	{
		return;
	}

	LOGI("alg_set_diff() diff=%d begin_table_id=%d end_table_id=%d\n",
		val->diff, val->begin_table_id, val->end_table_id);

	if (val->diff < MAX_DIFF_NUM)
	{
		for (i = val->begin_table_id; i <= val->end_table_id; i++)
		{
			tableId = i;

			gAlgData[tableId].diff = val->diff;
			table_clear(tableId);
		}
	}
	else
	{
		ret->result = 0;
	}
}


void alg_set_jackpot_rate(STR_AlgSetJackpotRate* val, STR_AlgSetJackpotRateResult* ret)
{
	int i;
	double multi = 10000.f;

	if (val->idx < 0 || val->idx >= TOTAL_JACKPOT_NUM)
	{
		LOGE("alg_set_jackpot_rate() error:  val->idx=%d\n", val->idx);
		ret->result = 0;
		return;
	}

	if (val->jackpot_rate >= multi)
	{
		LOGE("alg_set_jackpot_rate() error:  val->jackpot_rate=%d\n", val->jackpot_rate);
		ret->result = 0;
		return;
	}

	LOGI("alg_set_jackpot_rate() jackpot_rate=%d idx=%d\n",
		val->jackpot_rate, val->idx);

	ret->result = 1;
}


void alg_setting(STR_AlgSetting* val, STR_AlgSettingResult* ret)
{
	s32 i;
	s32 tableId;
	
	ret->result = 1;

	if (val->begin_table_id < 0 || val->begin_table_id >= MAX_TABLES_NUM)
	{
		val->begin_table_id = DEFAULT_TABLE_ID;
		/*LOGE("function: %s, Line: %d, val->begin_table_id error: %d\n", __FUNCTION__, __LINE__, val->begin_table_id);
		ret->result = 0;*/
	}

	if (val->end_table_id < 0 || val->end_table_id >= MAX_TABLES_NUM)
	{
		val->end_table_id = DEFAULT_TABLE_ID;
		/*LOGE("function: %s, Line: %d, val->end_table_id error: %d\n", __FUNCTION__, __LINE__, val->end_table_id);
		ret->result = 0;*/
	}

	if (val->begin_table_id > val->end_table_id)
	{
		LOGE("function: %s, Line: %d, val->begin_table_id(%d) < val->end_table_id(%d) error\n", __FUNCTION__, __LINE__, val->begin_table_id, val->end_table_id);
		ret->result = 0;
	}

	if (val->diff < 0 || val->diff >= MAX_DIFF_NUM)
	{
		LOGE("function: %s, Line: %d, val->diff error: %d\n", __FUNCTION__, __LINE__, val->diff);
		ret->result = 0;
	}

	if (val->coin_rate <= 0)
	{
		LOGE("function: %s, Line: %d, val->coin_rate error: %d\n", __FUNCTION__, __LINE__, val->coin_rate);
		ret->result = 0;
	}

	if (ret->result == 0)
	{
		return;
	}

	LOGI("alg_setting() diff=%d max_send=%d coin_rate=%d begin_table_id=%d end_table_id=%d\n",
		val->diff, val->max_send, val->coin_rate, val->begin_table_id, val->end_table_id);

	for (i = val->begin_table_id; i <= val->end_table_id; i++)
	{
		tableId = i;

		gAlgData[tableId].diff = val->diff;
		gAlgData[tableId].max_send = val->max_send;
		gAlgData[tableId].coin_rate = val->coin_rate;
		table_clear(tableId);
	}

	ret->result = 1;
}

void alg_set_config(STR_AlgSetConfig* val, STR_AlgSetConfigResult* ret)
{
	cJSON *jsonData = NULL;
	cJSON *itemJson = NULL;
	char *end = NULL;
	char jsonName[3] = {'a', 'a', '\0'};
	int i;
    
	jsonData = cJSON_ParseWithOpts(val->content, (const char **)&end, 0);
	if(jsonData == NULL)
	{
		LOGE("mammonSetConfig_Error: alg_set_config() jsonData is null");
		return;
	}

	ret->result = 1;
	ret->config = NULL;

	cJSON_Delete(jsonData);
}


int real_bi_bei(double bet, double win)
{
	if (bet == 0)
	{
		LOGE("function: %s, Line: %d, bet error: %lf\n", __FUNCTION__, __LINE__, bet);
		return 0;
	}

	//todo

	return 0;
}

void alg_bi_bei(STR_AlgBiBei* val, STR_AlgBiBeiResult* ret)
{
	double bet;
	double win;
	int result;

	bet = (double)val->bet / val->times;
	win = (double)(val->multi * val->bet) / val->times;
	result = real_bi_bei(bet, win);

	if(result)
		ret->result = 1;
	else
		ret->result = 0;
}

