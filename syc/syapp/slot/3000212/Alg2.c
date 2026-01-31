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

//#define ALG_CODE_FLOW			1
//#define _BIG_PRIZE_TEST_

STR_AlgData gAlgData[MAX_TABLES_NUM];

int port;

//缓存STR_AlgKillFishResult信息，以节省开辟内存时间
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

int get_totalWinMulti(s8 prizeType, int normalMulti, int *freegameMultis, s16 freegameNum)
{
	int totalMulti;
	int i;

	totalMulti = normalMulti;
	#ifdef DEBUG_PAYOUT_DIST
	printf("base game totalMulti:%d, %s\n", totalMulti, __FUNCTION__);
	#endif
	if(prizeType == E_PRIZETYPE_FREEGAME)  //disable free game
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
	target->animeType = source->animeType;
	target->totalMulti = source->totalMulti;

	target->normalIcons = source->normalIcons;
	target->normalMulti = source->normalMulti;
	target->normalLineInfos = source->normalLineInfos;

	if(source->prizeType == E_PRIZETYPE_FREEGAME)    
	{
		target->freegameIcons = source->freegameIcons;
		target->freegameMultis = source->freegameMultis;
		//target->freegameLineInfos = source->freegameLineInfos;
		#ifdef CAL_PRIZE_NUM
		target->prizeNumMulti = source->prizeNumMulti;
		#endif
	}

	target->normalLineNum = source->normalLineNum;
	target->freegameNum = source->freegameNum; 
	target->freegameLineNum = source->freegameLineNum;
	#ifdef CAL_PRIZE_NUM
	//target->przSymbolDist = source->przSymbolDist;
	//target->przFgSymbolDist = source->przFgSymbolDist;
	#endif
}

#ifdef OUTPUT_TEST
void test_function(STR_AlgKillFishResultTemp* ret)
{
	int i, j, k;
	printf("==================self test function start==================\n");
	printf("parameter 1\n");
	for(i = 0; i < BOARD_ICON_NUM; i++){
		printf("normalIcons[%d]:%d, line:%d test_function\n", i, ret->normalIcons[i], __LINE__);
	}
	printf("parameter 2\n");
	printf("normalLineNum:%ld, line:%d test_function\n", ret->normalLineNum, __LINE__);
	printf("parameter 3\n");
	for(i = 0; i < ret->normalLineNum; i++){
		for(j = 0; j < 3; j++)
			printf("normalLineInfos[%d][%d]:%d, line:%d test_function\n", i, j, ret->normalLineInfos[i][j], __LINE__);
	}
	printf("parameter 4\n");
	printf("normalMulti:%ld, line:%d test_function\n", ret->normalMulti, __LINE__);
	printf("parameter 5\n");
	printf("totalMulti:%ld, line:%d test_function\n", ret->totalMulti, __LINE__);
	printf("parameter 6\n");
	printf("animeType:%d, line:%d test_function\n", ret->animeType, __LINE__);	
	printf("parameter 7\n");
	printf("prizeType:%d, line:%d test_function\n", ret->prizeType, __LINE__);	
	printf("parameter 8\n");
	printf("freegameNum:%d, line:%d test_function\n", ret->freegameNum, __LINE__);
	printf("parameter 9\n");
	for(k = 0; k < gResult.freegameNum; k++)
		printf("freegameLineNum[%d]:%d, line:%d test_function\n", k, ret->freegameLineNum[k], __LINE__);
	printf("parameter 10\n");
	printf("freegameNum:%d, line:%d test_function\n", ret->freegameNum, __LINE__);
	if(ret->prizeType == E_PRIZETYPE_FREEGAME){	
		printf("parameter 11\n");
		for(k = 0; k < gResult.freegameNum; k++){
			for(i = 0; i < BOARD_ICON_NUM; i++)
				printf("freegameIcons[%d][%d]:%d, line:%d test_function\n", k, i, ret->freegameIcons[k][i], __LINE__);
			
		}
		printf("parameter 12\n");
		for(k = 0; k < gResult.freegameNum; k++)
			printf("freegameMultis[%d]:%d, line:%d test_function\n", k, ret->freegameMultis[k], __LINE__);
	}
	printf("==================self test function end==================\n");
}
#endif

void alg_kill_fish(STR_AlgKillFish* val, STR_AlgKillFishResult* ret)
{
	s32 tableId;
	double singleLineBet;
	double totalBet, totalWin;
	int diff, isSelect_A;
	gResult.animeType = 0;

	if (val->tableId < 0 || val->tableId >= MAX_TABLES_NUM)
	{
		val->tableId = DEFAULT_TABLE_ID;
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
	
	if (val->blackDiff < 0 || val->blackDiff >= MAX_DIFF_NUM)
	{
		if(val->blackDiff == -100)
			val->blackDiff = NORMAL_DIFF;
		else
			val->blackDiff = MAX_DIFF_NUM - 1;
	}

	if(val->playerDiff >= MAX_DIFF_NUM)
	{
		val->playerDiff = MAX_DIFF_NUM - 1;
	}

	tableId = val->tableId;
	singleLineBet = (double)val->lineBet / val->times;

	//cal in
	totalBet = singleLineBet * val->lineNum;
	table_in(tableId, totalBet);
	
	//difficulty set
	int loopTime = 2;        //最多循环两次
	//int isSelect_A = 1;
	isSelect_A = selectWeight(val->uid);
	diff = val->playerDiff >= 0 ? val->playerDiff : val->blackDiff;
	const int PRIZE_PROB_SUM_BG = isSelect_A > 0 ? PRIZE_PROB_SUM_BG_A : PRIZE_PROB_SUM_BG_B;
	const int FREEGAME_THRESHOLD = isSelect_A > 0 ? FREEGAME_THRESHOLD_A : FREEGAME_THRESHOLD_B;
	const int ANIME_THRESHOLD = isSelect_A > 0 ? ANIME_THRESHOLD_A : ANIME_THRESHOLD_B;
	const uint32_t *gBGPrizeSymbolWeight = isSelect_A > 0 ? gBGPrizeSymbolWeight_A : gBGPrizeSymbolWeight_B;
	const uint32_t *gFGPrizeSymbolWeight = isSelect_A > 0 ? gFGPrizeSymbolWeight_A : gFGPrizeSymbolWeight_B;
	
	while (loopTime)
	{
		//logic
		gResult.prizeType = board_calPrizeType(FREEGAME_THRESHOLD);//decide how to enter free game
		if(gResult.prizeType == E_PRIZETYPE_NORMAL){
			gResult.animeType = calAnimeType(ANIME_THRESHOLD);
			#ifdef DEBUG_PAYOUT_DIST
			printf("gResult.animeType:%d, line:%d in alg_kill_fish()\n", gResult.animeType, __LINE__);
			#endif
			board_gen(gRollerIcon, gRollerIconNum, gResult.normalIcons, gBGPrizeSymbolWeight, PRIZE_PROB_SUM_BG);
			gResult.normalMulti = board_calMulti(val->lineNum, gResult.normalIcons, gResult.normalLineInfos, &gResult.normalLineNum);
			#ifdef DEBUG_PAYOUT_DIST
			printf("gResult.normalMulti:%d, line:%d in alg_kill_fish()\n", gResult.normalMulti, __LINE__);
			#endif
		}
		
		if(gResult.prizeType == E_PRIZETYPE_FREEGAME){//disable free game
			int round, j, num;
			gResult.animeType = 1;
			#ifdef DEBUG_PAYOUT_DIST
			printf("gResult.animeType:%d, line:%d in alg_kill_fish()\n", gResult.animeType, __LINE__);
			#endif
			gResult.freegameNum = FREE_GAME_NUM;
			freeGame_gen(gResult.freegameNum, gResult.freegameIcons, gFGPrizeSymbolWeight, PRIZE_PROB_SUM_FG);
			//freeGame_gen(gResult.freegameNum, gResult.freegameIcons, gResult.przFgSymbolDist); //calculate_prize_symbol dist
			//freeGame_gen(gResult.freegameNum, gResult.freegameIcons, gResult.przFgSymbolDist, gResult.przFgNumDist); //calculate_prize_symbol and calculate_prize_num dist
			//freeGame_gen(gResult.freegameNum, gResult.freegameIcons, gResult.przFgNumDist); //calculate_prize_num dist
			
			for(round = 0; round < gResult.freegameNum; round++)
			{
				//if(board_calPrizeNum(gResult.freegameIcons[round], E_ICON_PRIZE) >= PRIZE_SYMBOL_THRESHOLD)
				//	gResult.freegameMultis[round] = board_calPrizeMulti(gResult.freegameIcons[round]);
				gResult.freegameMultis[round] = board_calMulti(0, gResult.freegameIcons[round], gResult.freegameLineInfos[round], &gResult.freegameLineNum[round]);
				#ifdef DEBUG_PAYOUT_DIST_FG
				printf("round:%d, gResult.freegameMultis[round]:%d, line:%d in alg_kill_fish()\n", round, gResult.freegameMultis[round], __LINE__);
				#endif
				#ifdef CAL_PRIZE_NUM
				if(board_calPrizeNum(gResult.freegameIcons[round], E_ICON_PRIZE) >= PRIZE_SYMBOL_THRESHOLD){//cal prize num multi dist
					num = board_calPrizeNum(gResult.freegameIcons[round], E_ICON_PRIZE);
					for(j = PRIZE_SYMBOL_THRESHOLD; j <= PRIZE_SYMBOL_NUM; j++){
						gResult.prizeNumMulti[round][j] = 0;//reset
						if(j == num){
							gResult.prizeNumMulti[round][j] = board_calPrizeMulti(gResult.freegameIcons[round]);
							#ifdef DEBUG_PAYOUT_DIST_FG
							printf("round:%d, j:%d, gResult.prizeNumMulti[round][j]:%d, line:%d in alg_kill_fish()\n", round, j, gResult.prizeNumMulti[round][j], __LINE__);
							#endif
						}
					}
				}
				#endif
			}
		}
	
		//maxSend
		gResult.totalMulti = get_totalWinMulti(gResult.prizeType, gResult.normalMulti, gResult.freegameMultis, gResult.freegameNum);
		totalWin = singleLineBet * gResult.totalMulti;
		#ifdef DEBUG_PAYOUT_DIST
		printf("gResult.totalMulti:%d , totalWin:%d line:%d in alg_kill_fish()\n", gResult.totalMulti, totalWin, __LINE__);
		#endif
		difficulty_control(val->lineNum, diff, 50, 0, gTargetRTP, isSelect_A, gResult.totalMulti, &loopTime);
	}
	#ifdef OUTPUT_TEST
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

	if (val->level < 0 || val->level > MAX_SEND_MAX_VALUE)
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

