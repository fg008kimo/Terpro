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
#include "logic/buff.h"

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

int get_totalWinMulti(s8 prizeType, u32 normalMulti, int *freegameMultis, s16 freegameNum, int *rotaryMultis, s16 rotaryNum)
{
	int totalMulti;
	int i;

	totalMulti = normalMulti;
	if(prizeType == E_PRIZETYPE_FREEGAME)
	{
		for(i = 0; i < freegameNum; i++)
			totalMulti += freegameMultis[i];
		for(i = 0; i < rotaryNum; i++)
			totalMulti += rotaryMultis[i];
	}
	return totalMulti;
}

void cpy_result(STR_AlgKillFishResult* target, STR_AlgKillFishResultTemp* source)
{
	target->prizeType = source->prizeType;
	target->totalMulti = source->totalMulti;
	target->normalIconsAgo = source->normalIconsAgo;
	target->normalIcons = source->normalIcons;
	target->normalMulti = source->normalMulti;

	if(source->prizeType == E_PRIZETYPE_FREEGAME)
	{
		target->freegameIconsAgo = source->freegameIconsAgo;
		target->freegameIcons = source->freegameIcons;
		target->freegameMultis = source->freegameMultis;
		target->rotaryMultis = source->rotaryMultis;
		target->buffCounter = source->buffCounter;
		target->hitTarget = source->hitTarget;
		target->rotaryData = source->rotaryData;
	}

	target->freegameNum = source->freegameNum;
	target->rotaryNum = source->rotaryNum;
}

void alg_kill_fish(STR_AlgKillFish* val, STR_AlgKillFishResult* ret)
{
	s32 tableId;
	double singleLineBet;
	double totalBet, totalWin;

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

	if (val->lineNum < 0 || val->lineNum > BET_LINE_TOTAL)//BET_LINE_TOTAL=75
	{
		LOGE("function: %s, Line: %d, val->lineNum error: %hd\n", __FUNCTION__, __LINE__, val->lineNum);
	}

	tableId = val->tableId;
	singleLineBet = (double)val->lineBet / val->times;

	//cal in
	totalBet = singleLineBet * val->lineNum;
	table_in(tableId, totalBet);//入金這個table

	//logic
	printf("####################start game##################\n");
	int reelPos = 0;
	if(is_dajiang_mode())//進入buy free game
		reelPos = gen_board(gBuyFreeGameRollerIcon, gBuyFreeGameRollerIconNum, gBaseReelWeight, BASE_REEL_NUM+4, gResult.normalIconsAgo);//reelPos是第幾條母輪帶
	else
		reelPos = gen_board(gRollerIcon, gRollerIconNum, gBaseReelWeight, BASE_REEL_NUM+4, gResult.normalIconsAgo);//gRollerIcon: base game背景輪帶, gRollerIconNum=各個子輪帶數目, gBaseReelWeight=各輪帶權重矩陣, gResult.normalIconsAgo = 符号替换前图标列表（一维数组）
	//但base game和buyFreeGame 的len都是BASE_REEL_NUM+4,所以一般牛不會轉成金牛
	
	gResult.normalMulti = 0;
	memcpy(gResult.normalIcons, gResult.normalIconsAgo, sizeof(gResult.normalIconsAgo));//把normalIconsAgo大小,且資料內容是gResult.normalIconsAgo的矩陣 copy到gResult.normalIcons(normalIcons是替換後的一維矩陣)
	normalIconsToBuff(reelPos, gResult.normalIcons);//在base game就有機率把其他動物轉成一般牛,reelPos 是可以轉成一般牛的symbol(看名稱是輪帶位置,但上個開發者懶得改)(只有 gBaseReelWeight 上的1~4條有機會被轉成一般牛)
	cal_boardMulti(E_PRIZETYPE_NORMAL, gResult.normalIcons, &gResult.normalMulti, E_ICON_NORMAL_SCAT);//整理盤面,把該換的圖標換掉 , gResult.normalMulti是結果倍率, E_ICON_NORMAL_SCAT = base game scatter
	gResult.prizeType = getPrizeType(gResult.normalIcons);
	
	if(gResult.prizeType == E_PRIZETYPE_FREEGAME)   //处理免费游戏
	{
		//转盘游戏 (轉盤遊戲也算FREE GAME,但是是五次內有機會直接進到真FREE GAME,超過五次沒轉到FREE GAME強制進入FREE GAME)
		int i, j, round;
		int rotaryRound = 0;
		memset(gResult.rotaryMultis, 0, sizeof(gResult.rotaryMultis));
		int rotaryIcon[ROTARY_ICON_NUM] = {1,2,3,4,5,6,7,8,9,10,11,12};//ROTARY_ICON_NUM = 12 (轉盤上的12個位置)
		while (rotaryRound < ROTARY_INIT_ROUND)//ROTARY_INIT_ROUND = 5 (rotaryRound = 0~4)
		{
			int reBonus[BONUS_NUM] = {0};//BONUS_NUM = 5
			for (i = 0; i < ROTARY_ICON_NUM; i++)//i=0~11
			{
				for (j = 0; j < gRotaryIconNum[1]; j++)//gRotaryIconNum[3] = {3,4,5};gRotaryIconNum[1] = 4, j=0~3
				{
					if(rotaryIcon[i] == gRotaryIcon[1][j])//gRotaryIcon[3][BONUS_NUM] = {{3,6,11},{1,4,7,9},{2,5,8,10,12}}, so gRotaryIcon[1][j] = {1,4,7,9}//金牛(gRotaryIcon = 轉盤icon的reference)
						gResult.rotaryData[rotaryRound][i] = rotaryRound+1;//rotaryData[ROTARY_INIT_ROUND][ROTARY_ICON_NUM] = rotaryData[0~4][0~11], rotaryRound+1 = 1~5
					printf("rotaryIcon[i]:%d, gResult.rotaryData[rotaryRound][i]:%d, rotaryRound:%d, gRotaryIcon[1][j]:%d, i:%d, j:%d\n", rotaryIcon[i], gResult.rotaryData[rotaryRound][i], rotaryRound, gRotaryIcon[1][j], i ,j);
				}
				for (j = 0; j < gRotaryIconNum[2]; j++)//RotaryIconNum[2]=5, j=0~4
				{
					if(rotaryIcon[i] == gRotaryIcon[2][j])//gRotaryIcon[2][j] = {2,5,8,10,12}//奖金
					{
						int bouns = GetRand(BONUS_NUM)+1;//BONUS_NUM=0~4, bonus=1~5
						//printf("bouns:%d\n",bouns);
						while (reBonus[bouns-1])//紀錄重複中獎項目矩陣,假設bonus = 4, reBonus[3]=1
							bouns = GetRand(BONUS_NUM)+1;
						reBonus[bouns-1] = 1;
						gResult.rotaryData[rotaryRound][i] = (bouns+rotaryRound)*val->lineNum;//val->lineNum = 75
						printf("rotaryIcon[i]:%d, gResult.rotaryData[rotaryRound][i]:%d, rotaryRound:%d, bouns:%d, gRotaryIcon[2][j]:%d, i:%d, j:%d\n", rotaryIcon[i], gResult.rotaryData[rotaryRound][i], rotaryRound, bouns, gRotaryIcon[2][j], i ,j);
					}
				}//上面兩個for迴圈在產生wheel盤面的背景數值
				/* raw data
				rotaryIcon[i]:1, gResult.rotaryData[rotaryRound][i]:1, rotaryRound:0, gRotaryIcon[1][j]:1, i:0, j:0
				rotaryIcon[i]:1, gResult.rotaryData[rotaryRound][i]:1, rotaryRound:0, gRotaryIcon[1][j]:4, i:0, j:1
				rotaryIcon[i]:1, gResult.rotaryData[rotaryRound][i]:1, rotaryRound:0, gRotaryIcon[1][j]:7, i:0, j:2
				rotaryIcon[i]:1, gResult.rotaryData[rotaryRound][i]:1, rotaryRound:0, gRotaryIcon[1][j]:9, i:0, j:3
				rotaryIcon[i]:2, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:1, i:1, j:0
				rotaryIcon[i]:2, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:4, i:1, j:1
				rotaryIcon[i]:2, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:7, i:1, j:2
				rotaryIcon[i]:2, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:9, i:1, j:3
				rotaryIcon[i]:2, gResult.rotaryData[rotaryRound][i]:300, rotaryRound:0, bouns:4, gRotaryIcon[2][j]:2, i:1, j:0
				rotaryIcon[i]:3, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:1, i:2, j:0
				rotaryIcon[i]:3, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:4, i:2, j:1
				rotaryIcon[i]:3, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:7, i:2, j:2
				rotaryIcon[i]:3, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:9, i:2, j:3
				rotaryIcon[i]:4, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:1, i:3, j:0
				rotaryIcon[i]:4, gResult.rotaryData[rotaryRound][i]:1, rotaryRound:0, gRotaryIcon[1][j]:4, i:3, j:1
				rotaryIcon[i]:4, gResult.rotaryData[rotaryRound][i]:1, rotaryRound:0, gRotaryIcon[1][j]:7, i:3, j:2
				rotaryIcon[i]:4, gResult.rotaryData[rotaryRound][i]:1, rotaryRound:0, gRotaryIcon[1][j]:9, i:3, j:3
				rotaryIcon[i]:5, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:1, i:4, j:0
				rotaryIcon[i]:5, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:4, i:4, j:1
				rotaryIcon[i]:5, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:7, i:4, j:2
				rotaryIcon[i]:5, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:9, i:4, j:3
				rotaryIcon[i]:5, gResult.rotaryData[rotaryRound][i]:150, rotaryRound:0, bouns:2, gRotaryIcon[2][j]:5, i:4, j:1
				rotaryIcon[i]:6, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:1, i:5, j:0
				rotaryIcon[i]:6, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:4, i:5, j:1
				rotaryIcon[i]:6, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:7, i:5, j:2
				rotaryIcon[i]:6, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:9, i:5, j:3
				rotaryIcon[i]:7, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:1, i:6, j:0
				rotaryIcon[i]:7, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:4, i:6, j:1
				rotaryIcon[i]:7, gResult.rotaryData[rotaryRound][i]:1, rotaryRound:0, gRotaryIcon[1][j]:7, i:6, j:2
				rotaryIcon[i]:7, gResult.rotaryData[rotaryRound][i]:1, rotaryRound:0, gRotaryIcon[1][j]:9, i:6, j:3
				rotaryIcon[i]:8, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:1, i:7, j:0
				rotaryIcon[i]:8, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:4, i:7, j:1
				rotaryIcon[i]:8, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:7, i:7, j:2
				rotaryIcon[i]:8, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:9, i:7, j:3
				rotaryIcon[i]:8, gResult.rotaryData[rotaryRound][i]:75, rotaryRound:0, bouns:1, gRotaryIcon[2][j]:8, i:7, j:2
				rotaryIcon[i]:9, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:1, i:8, j:0
				rotaryIcon[i]:9, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:4, i:8, j:1
				rotaryIcon[i]:9, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:7, i:8, j:2
				rotaryIcon[i]:9, gResult.rotaryData[rotaryRound][i]:1, rotaryRound:0, gRotaryIcon[1][j]:9, i:8, j:3
				rotaryIcon[i]:10, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:1, i:9, j:0
				rotaryIcon[i]:10, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:4, i:9, j:1
				rotaryIcon[i]:10, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:7, i:9, j:2
				rotaryIcon[i]:10, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:9, i:9, j:3
				rotaryIcon[i]:10, gResult.rotaryData[rotaryRound][i]:375, rotaryRound:0, bouns:5, gRotaryIcon[2][j]:10, i:9, j:3
				rotaryIcon[i]:11, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:1, i:10, j:0
				rotaryIcon[i]:11, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:4, i:10, j:1
				rotaryIcon[i]:11, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:7, i:10, j:2
				rotaryIcon[i]:11, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:9, i:10, j:3
				rotaryIcon[i]:12, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:1, i:11, j:0
				rotaryIcon[i]:12, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:4, i:11, j:1
				rotaryIcon[i]:12, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:7, i:11, j:2
				rotaryIcon[i]:12, gResult.rotaryData[rotaryRound][i]:0, rotaryRound:0, gRotaryIcon[1][j]:9, i:11, j:3
				rotaryIcon[i]:12, gResult.rotaryData[rotaryRound][i]:225, rotaryRound:0, bouns:3, gRotaryIcon[2][j]:12, i:11, j:4
				*/
				
				#ifdef PRINT
				//printf("%d(%d)\t", rotaryIcon[i], gResult.rotaryData[rotaryRound][i]);
				//printf("rotaryIcon[i]:%d, gResult.rotaryData[rotaryRound][i]:%d, rotaryRound:%d\n", rotaryIcon[i], gResult.rotaryData[rotaryRound][i], rotaryRound);
				#endif
			}
		
			//将出现的金牛符个数进行累加到计数器中
			int hitTarget = getRandomValue(hitWeight, ROTARY_ICON_NUM, WILD_WEIGHT)-1;//hitWeight = {3,10},{6,10},{11,10},{1,5},{4,5},{7,5},{9,5},{2,10},{5,10},{8,10},{10,10},{12,10} (hitTarget是wheel指針轉到的值)
			printf("hitTarget:%d, rotaryRound:%d\n", hitTarget, rotaryRound);//怎麼取hitTarget去getRandomValue()看
			gResult.hitTarget[rotaryRound] = hitTarget;
			if(gResult.rotaryData[rotaryRound][hitTarget] < BUFF_BONUS_BOUNDARY)//BUFF_BONUS_BOUNDARY = 6(所以金牛上限是5) (<6代表你轉到金牛(1~5),此時紀錄金牛數量)
				gResult.buffCounter += gResult.rotaryData[rotaryRound][hitTarget];//buffCounter = 金牛计数器 (gResult.rotaryData[rotaryRound][hitTarget]要去對此次wheel的背景盤面產生出來的值)
			printf("gResult.buffCounter:%d\n", gResult.buffCounter);
			if(gResult.rotaryData[rotaryRound][hitTarget] > BUFF_BONUS_BOUNDARY)//(>6代表你轉到獎金,此時紀錄獎金分數)
				gResult.rotaryMultis[rotaryRound] = gResult.rotaryData[rotaryRound][hitTarget];//rotaryMultis = 转盘倍率
			printf("gResult.rotaryMultis[rotaryRound]:%d\n", gResult.rotaryMultis[rotaryRound]);
			//如果轉到FREE GAME就直接跳FREE GAAME 這邊就先不記錄數值
			//printf("gResult.buffCounter:%d, gResult.rotaryMultis[rotaryRound]:%d\n", gResult.buffCounter, gResult.rotaryMultis[rotaryRound]);
	
			if(gResult.rotaryData[rotaryRound][hitTarget] == 0)
			{
				rotaryRound++;
				break;
			}
			rotaryRound++;
			printf("rotaryRound:%d\n", rotaryRound);
		}
		gResult.rotaryNum = rotaryRound;//rotaryNum紀錄剛剛輪盤轉了幾次

		int scatNum = calIconNum(gResult.normalIcons, E_ICON_NORMAL_SCAT);//E_ICON_NORMAL_SCAT = 2/3/4/5, scatNum = Number of free game = 5/8/15/20 (這裡是免費遊戲)
		int freeTotalRound = gFreeGameInfo[scatNum-2][1];//gFreeGameInfo[FREEGAME_TYPE][2] = {{2,5},{3,8},{4,15},{5,20}}, scatNum-2 = 0 1 2 3, freeTotalRound = 5 8 15 20其中一個
		for (round = 0; round < freeTotalRound; round++)
		{
			printf("round:%d in free game, freeTotalRound:%d\n", round, freeTotalRound);
			gResult.freegameMultis[round] = 0;
			gen_board(gFreeGameRollerIcon, gFreeGameRollerIconNum, gFreeReelWeight, BASE_REEL_NUM, gResult.freegameIconsAgo[round]);
			memcpy(gResult.freegameIcons[round], gResult.freegameIconsAgo[round], sizeof(gResult.freegameIcons[round]));
			gResult.buffCounter += calIconNum(gResult.freegameIcons[round], E_ICON_BUFF);//計算盤面上金牛個數
			freeIconsToBuff(gResult.freegameIcons[round], gResult.buffCounter);//在free game 金牛會替換成一般牛
			cal_boardMulti(gResult.prizeType, gResult.freegameIcons[round], &gResult.freegameMultis[round], E_ICON_FREE_SCAT);//E_ICON_FREE_SCAT = freeGame scatter

			scatNum = calIconNum(gResult.freegameIcons[round], E_ICON_FREE_SCAT);
			if(scatNum >= gFreeGameInfo[0][0])
			{
				// 免费又免费
				freeTotalRound += gFreeGameInfo[scatNum-2][1];
			}
		}
		
		gResult.buffCounter = 0;
		gResult.freegameNum = freeTotalRound;
	}

	//maxSend
	gResult.totalMulti = get_totalWinMulti(gResult.prizeType, gResult.normalMulti, gResult.freegameMultis, gResult.freegameNum, gResult.rotaryMultis, gResult.rotaryNum);
	totalWin = singleLineBet * gResult.totalMulti;
	printf("####################end game##################\n");

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