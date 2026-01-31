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

#ifdef DEBUG_MODE
	#define ALG_CODE_FLOW			1
	#define OUTPUT_TEST    			1
#endif	
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
	#ifdef ALG_CODE_FLOW
	printf("betCoins:%f, gAlgData[tableId].gRealInOutCoins[0]:%f, gAlgData[tableId].coin_rate:%f, line:%d in table_in()\n", betCoins, gAlgData[tableId].gRealInOutCoins[0], gAlgData[tableId].coin_rate, __LINE__);
	#endif
}

void table_out(s32 tableId, double winScore)
{
	double winCoins;

	winCoins = winScore / gAlgData[tableId].coin_rate;
	gAlgData[tableId].gRealInOutCoins[1] += winCoins;
	#ifdef ALG_CODE_FLOW
	printf("winCoins:%f, gAlgData[tableId].gRealInOutCoins[1]:%f, gAlgData[tableId].coin_rate:%f, line:%d in table_out()\n", winCoins, gAlgData[tableId].gRealInOutCoins[1], gAlgData[tableId].coin_rate, __LINE__);
	#endif
}

void table_clear(s32 tableId)
{
	gAlgData[tableId].gRealInOutCoins[0] = 0;
	gAlgData[tableId].gRealInOutCoins[1] = 0;
}

s64 get_totalWinMulti(s8 prizeType, s64 normalMulti, s64 freegameMultis, s64 respinMulti)
{
	s64 totalMulti = 0;

	#ifdef ALG_CODE_FLOW
	printf("normalMulti:%ld, freegameMultis:%ld, respinMulti:%ld, line:%d in get_totalWinMulti()\n", normalMulti, freegameMultis, respinMulti, __LINE__);
	#endif
	totalMulti = normalMulti;
	if(prizeType == E_PRIZETYPE_RESPINGAME)
		totalMulti += respinMulti;	
	if(prizeType == E_PRIZETYPE_FREEGAME)
		totalMulti += freegameMultis;
	normalMulti = freegameMultis = respinMulti = 0;

	return totalMulti;
}

void cpy_result(STR_AlgKillFishResult* target, STR_AlgKillFishResultTemp* source)
{
	target->prizeType = source->prizeType;
	target->set = source->set;
	target->totalMulti = source->totalMulti;

	target->normalIcons = source->normalIcons;
	target->normalpreIcons = source->normalpreIcons;
	target->respinIcons = source->respinIcons;
	target->normalMulti = source->normalMulti;
	
	//if(source->prizeType == E_PRIZETYPE_RESPINGAME)
	//{
	target->respinMulti = source->respinMulti;
	//}
	//if(source->prizeType == E_PRIZETYPE_FREEGAME)
	//{
	target->freegameMultis = source->freegameMultis;
	//}
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
	for(i = 0; i < BOARD_ICON_NUM; i++){
		printf("normalpreIcons[%d]:%d, line:%d test_function\n", i, ret->normalpreIcons[i], __LINE__);
	}
	printf("parameter 3\n");
	printf("normalMulti:%ld, line:%d test_function\n", ret->normalMulti, __LINE__);
	printf("parameter 4\n");
	printf("totalMulti:%ld, line:%d test_function\n", ret->totalMulti, __LINE__);
	printf("parameter 5\n");
	printf("set:%d, line:%d test_function\n", ret->set, __LINE__);	
	printf("parameter 6\n");
	printf("prizeType:%d, line:%d test_function\n", ret->prizeType, __LINE__);	
	//if(ret->prizeType == E_PRIZETYPE_RESPINGAME){	
	printf("parameter 7\n");
	printf("respinMulti:%ld, line:%d test_function\n", ret->respinMulti, __LINE__);
	printf("parameter 8\n");
	for(i = 0; i < BOARD_ICON_NUM; i++){
		printf("respinIcons[%d]:%d, line:%d test_function\n", i, ret->respinIcons[i], __LINE__);
	}
	//}
	//if(ret->prizeType == E_PRIZETYPE_FREEGAME){	
	printf("parameter 9\n");
	printf("freegameMultis:%ld, line:%d test_function\n", ret->freegameMultis, __LINE__);
	//}
	printf("==================self test function end==================\n");
}
#endif

void alg_kill_fish(STR_AlgKillFish* val, STR_AlgKillFishResult* ret)
{
	s32 tableId;
	double totalBet, totalWin;
	int isSelect_A, diff, option, i;
	s64 testNormal; 
	
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

	if (val->lineNum < 0)
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
	//reset
	gResult.normalMulti = gResult.respinMulti = gResult.freegameMultis = gResult.set = 0;
	memset(gResult.normalIcons, 0, sizeof(int)*BOARD_ICON_NUM);
	memset(gResult.respinIcons, 0, sizeof(int)*BOARD_ICON_NUM);
	memset(gResult.normalpreIcons, 0, sizeof(int)*BOARD_ICON_NUM);

	tableId = val->tableId;
	diff = val->playerDiff >= 0 ? val->playerDiff : val->blackDiff;
	//cal in
	totalBet = (double)val->lineBet * val->lineNum / val->times;
	#ifdef ALG_CODE_FLOW
	printf("val->times:%d, totalBet:%f, line:%d in alg_kill_fish()\n", val->times, totalBet, __LINE__);
	#endif
	table_in(tableId, totalBet * val->times);
	gResult.prizeType = E_PRIZETYPE_NORMAL;
	
	//switch setting
	const uint16_t (*gRollerIconNum)[BOARD_COLUMN];
	const uint8_t (*gRollerIcon)[BOARD_COLUMN][ROLLER_ICON_LEN];
	const uint32_t (*gTableWeight)[TABLE_NUM][2];
	const uint32_t (*gBaseSymbolWeight)[TABLE_NUM][BOARD_COLUMN][ICON_NUM][2];
	const uint32_t (*gBaseSymbolWeightNum)[TABLE_NUM][BOARD_COLUMN];
	const uint32_t (*gMulWeight)[MUL_NUM][2];
	const uint32_t (*gRotaryIcon)[ROTARY_ICON_NUM][2];
	const double (*gTargetRTP)[MAX_DIFF_NUM][2];
	if(val->set == 1){
		gResult.set = 1;
		gRollerIconNum = gRollerIconNum1;
		gRollerIcon = gRollerIcon1;
		gTableWeight = gTableWeight1;
		gBaseSymbolWeight = gBaseSymbolWeight1;
		gBaseSymbolWeightNum = gBaseSymbolWeightNum1;
		gMulWeight = gMulWeight1;
		gRotaryIcon = gRotaryIcon1;
		gTargetRTP = gTargetRTP1;
		if(totalBet == 0.1) option = 0;
		else if(totalBet == 0.5) option = 1;
		else if(totalBet == 1) option = 2;
		else if(totalBet == 5) option = 3;
		else if(totalBet == 10) option = 4;
		else LOGE("function: %s, Line: %d, totalBet error: %f\n", __FUNCTION__, __LINE__, totalBet);
	}else if(val->set == 2){
		gResult.set = 2;
		gRollerIconNum = gRollerIconNum2;
		gRollerIcon = gRollerIcon2;
		gTableWeight = gTableWeight2;
		gBaseSymbolWeight = gBaseSymbolWeight2;
		gBaseSymbolWeightNum = gBaseSymbolWeightNum2;
		gMulWeight = gMulWeight2;
		gRotaryIcon = gRotaryIcon2;
		gTargetRTP = gTargetRTP2;
		if(totalBet == 1) option = 0;
		else if(totalBet == 5) option = 1;
		else if(totalBet == 10) option = 2;
		else if(totalBet == 50) option = 3;
		else if(totalBet == 100) option = 4;
		else LOGE("function: %s, Line: %d, totalBet error: %f\n", __FUNCTION__, __LINE__, totalBet);
	}else if(val->set == 3){
		gResult.set = 3;
		gRollerIconNum = gRollerIconNum3;
		gRollerIcon = gRollerIcon3;
		gTableWeight = gTableWeight3;
		gBaseSymbolWeight = gBaseSymbolWeight3;
		gBaseSymbolWeightNum = gBaseSymbolWeightNum3;
		gMulWeight = gMulWeight3;
		gRotaryIcon = gRotaryIcon3;
		gTargetRTP = gTargetRTP3;
		if(totalBet == 10) option = 0;
		else if(totalBet == 50) option = 1;
		else if(totalBet == 100) option = 2;
		else if(totalBet == 500) option = 3;
		else if(totalBet == 1000) option = 4;
		else LOGE("function: %s, Line: %d, totalBet error: %f\n", __FUNCTION__, __LINE__, totalBet);
	}else if(val->set == 4){
		gResult.set = 4;
		gRollerIconNum = gRollerIconNum4;
		gRollerIcon = gRollerIcon4;
		gTableWeight = gTableWeight4;
		gBaseSymbolWeight = gBaseSymbolWeight4;
		gBaseSymbolWeightNum = gBaseSymbolWeightNum4;
		gMulWeight = gMulWeight4;
		gRotaryIcon = gRotaryIcon4;
		gTargetRTP = gTargetRTP4;
		if(totalBet == 100) option = 0;
		else if(totalBet == 500) option = 1;
		else if(totalBet == 1000) option = 2;
		else if(totalBet == 5000) option = 3;
		else if(totalBet == 10000) option = 4;
		else LOGE("function: %s, Line: %d, totalBet error: %f\n", __FUNCTION__, __LINE__, totalBet);
	}
	#ifdef ALG_CODE_FLOW
	printf("option:%d, val->set:%d, line:%d in alg_kill_fish()\n", option, val->set, __LINE__);
	#endif

	//logic
	int loopTime = 2;
	isSelect_A = 1;
	while (loopTime){
		board_gen(gTableWeight, TABLE_NUM - 1, gBaseSymbolWeight, gBaseSymbolWeightNum, gMulWeight, gRollerIcon, gRollerIconNum, gResult.normalIcons, 0, option);
		gResult.normalMulti = board_calMulti(gResult.normalIcons, val->times);
		testNormal = gResult.normalMulti;
		
		#ifdef ALG_CODE_FLOW
		printf("gResult.normalMulti:%ld, line:%d in alg_kill_fish()\n", gResult.normalMulti, __LINE__);
		#endif
		if(((gResult.normalIcons[10] == E_ICON_SC1) && (option > 0) && (option < OPTION_NUM)) || 
			(gResult.normalIcons[10] == E_ICON_SC2) && (option > 0) && (option < OPTION_NUM))
			gResult.prizeType = E_PRIZETYPE_FREEGAME;
		else if(gResult.normalIcons[10] == E_ICON_RE){
			gResult.prizeType = E_PRIZETYPE_RESPINGAME;
			int respin = 1;
			//logic
			board_gen(0, 0, gBaseSymbolWeight, gBaseSymbolWeightNum, 0, gRollerIcon, gRollerIconNum, gResult.respinIcons, respin, option);
			gResult.respinMulti = board_calMulti(gResult.respinIcons, val->times);
			#ifdef ALG_CODE_FLOW
			printf("gResult.respinMulti:%ld, line:%d in alg_kill_fish()\n", gResult.respinMulti, __LINE__);
			#endif
			gResult.normalMulti = testNormal;
			#ifdef ALG_CODE_FLOW
			printf("gResult.normalMulti:%ld, line:%d in alg_kill_fish()\n", gResult.normalMulti, __LINE__);
			#endif
			if(gResult.respinIcons[10] != E_ICON_RE){//perform used
				gResult.respinIcons[10] = E_ICON_RE;
				gResult.respinIcons[9] = gResult.respinIcons[11] = E_ICON_PIPCEMPTY;
			}
		}
		if((gResult.prizeType == E_PRIZETYPE_FREEGAME) && (option > 0) && (option < OPTION_NUM))
		{
			int rotaryMulti = 0, sc = 0;
			if((option == 1) || (option == 2))
				sc = 1;
			else if((option == 3) || (option == 4))
				sc = 2;
			
			rotaryMulti = getRandomValue(gRotaryIcon[sc], ROTARY_ICON_NUM, ROTARY_WEIGHT_SUM); //gen the number chosen from rotary board
			#ifdef ALG_CODE_FLOW
			int i;
			for(i = 0; i < ROTARY_ICON_NUM; i++)
				printf("sc:%d, i:%d, gRotaryIcon[sc][i][0]:%d, gRotaryIcon[sc][i][1]:%d, line:%d in alg_kill_fish()\n", sc, i, gRotaryIcon[sc][i][0], gRotaryIcon[sc][i][1], __LINE__);
			printf("rotaryMulti:%d, line:%d in alg_kill_fish()\n", rotaryMulti, __LINE__);
			#endif
			//#ifdef ALG_CODE_FLOW
			//printf("rotaryMulti:%d, line:%d in alg_kill_fish()\n", rotaryMulti, __LINE__);
			//#endif
			gResult.freegameMultis = rotaryMulti * val->lineBet * val->lineNum; 
			#ifdef ALG_CODE_FLOW
			printf("gResult.freegameMultis:%ld, line:%d in alg_kill_fish()\n", gResult.freegameMultis, __LINE__);
			printf("gResult.normalMulti:%ld, line:%d in alg_kill_fish()\n", gResult.normalMulti, __LINE__);
			#endif
		}
	
		//maxSend
		testNormal = 0;
		gResult.totalMulti = get_totalWinMulti(gResult.prizeType, gResult.normalMulti, gResult.freegameMultis, gResult.respinMulti);
		//perform
		if(gResult.totalMulti >= val->lineBet * val->lineNum * 10)
			board_genPerform(gPerformAction, gPerformPosition, PERFORM_NUM, gRollerIcon, gRollerIconNum, gResult.normalpreIcons, gResult.normalIcons, option, gResult.normalMulti);
		else
			memcpy(gResult.normalpreIcons, gResult.normalIcons, sizeof(int)*BOARD_ICON_NUM);
		#ifdef ALG_CODE_FLOW
		printf("gResult.totalMulti:%ld, line:%d in alg_kill_fish()\n", gResult.totalMulti, __LINE__);
		#endif
		totalWin = gResult.totalMulti;
		difficulty_control(diff, 50, 0, gTargetRTP, isSelect_A, gResult.totalMulti, &loopTime, option);
	}
	#ifdef OUTPUT_TEST
	test_function(&gResult);
	#endif

	//cal out
	table_out(tableId, totalWin);

	//result
	cpy_result(ret, &gResult);
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

