/*
除了个别要做数组下标的宏定义放data.h，其它配置统一放data.c，
*/

#ifndef _ALG_DATA_H_
#define _ALG_DATA_H_

#include <stdint.h>

extern const int ALG_VERSION;
extern const int ALG_VERSION_BIG_PRIZE;

#define BET_LINE_TOTAL      		10               //最多压线数
#define MAX_DIFF_NUM        		10               //难度个数
#define SELECT_VERSION_NUM  		9                //可供选择的版本
#define VERSION_WEIGHT      		10000            //可供选择版本的总权重
#define MAX_SEND_MAX_VALUE  		100000000        //最大炒场
#define ICON_NUM            		8                //图标种类
#define BOARD_ROW           		3                //盘面行数
#define BOARD_COLUMN        		3                //盘面列数
#define BOARD_ICON_NUM      		10               //盘面图标数量
#define ROLLER_ICON_LEN     		300              //每条滚轮图标最大数量
#define FREE_GAME_NUM	    		8              	 //免費遊戲場數
#define PRIZE_SYMBOL_NUM			10               //得獎符數目
 
#define PRIZE_PROB_SUM_BG_A	    	100000         	//Prize symbol weight in base game
#define FREEGAME_THRESHOLD_A		9763928			//97639282943365  
#define ANIME_THRESHOLD_A			49300			  

#define PRIZE_PROB_SUM_BG_B	    	10000         	//Prize symbol weight in base game
#define FREEGAME_THRESHOLD_B		1995039			//19950395433475	
#define ANIME_THRESHOLD_B			99995				

#define PRIZE_PROB_SUM_FG	    	10000         	 	//Prize symbol weight in free game
#define PRIZE_SYMBOL_THRESHOLD		5

#define PRIZE_NUM_PROB_SUM	    	1000

#define DEBUG_MODE						1
#ifdef DEBUG_MODE
	#define DEBUG_PAYOUT_DIST			1
	#define PRINT_BOARD 				1   	
	#define OUTPUT_TEST 				1   	
#endif	

//#define disable_freegame			1
#define CAL_PRIZE_NUM    			1



extern const int E_ICON_NULL;
extern const int E_ICON_PIPC1;
extern const int E_ICON_PIPC2;
extern const int E_ICON_PIPC3;
extern const int E_ICON_PIPC4;
extern const int E_ICON_PIPC5;
extern const int E_ICON_PIPC6;
extern const int E_ICON_WILD;
extern const int E_ICON_PRIZE;

extern const int E_PRIZETYPE_NORMAL;
extern const int E_PRIZETYPE_FREEGAME;
extern const int NORMAL_DIFF;

extern const uint8_t gLinePos[BET_LINE_TOTAL][BOARD_COLUMN];

extern const uint16_t gIconMulti[ICON_NUM+1][BOARD_COLUMN+1];
extern const uint32_t gPrizeSymbol[PRIZE_SYMBOL_NUM][2];
extern const uint32_t gBGPrizeSymbolWeight_A[PRIZE_SYMBOL_NUM];
extern const uint32_t gBGPrizeSymbolWeight_B[PRIZE_SYMBOL_NUM];
extern const uint32_t gFGPrizeSymbolWeight_A[PRIZE_SYMBOL_NUM];
extern const uint32_t gFGPrizeSymbolWeight_B[PRIZE_SYMBOL_NUM];
extern const uint32_t gPrizeNumWeight[PRIZE_SYMBOL_NUM];

extern const uint16_t gRollerIconNum[BOARD_COLUMN];
extern const uint8_t gRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN];

extern const uint16_t gFreeGameRollerIconNum[BOARD_COLUMN];
extern const uint8_t gFreeGameRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN];

extern const double gTargetRTP[10][3];
extern const int gSelectVersion[SELECT_VERSION_NUM][2];

#endif
