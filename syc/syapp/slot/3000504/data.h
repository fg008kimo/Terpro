/*
除了个别要做数组下标的宏定义放data.h，其它配置统一放data.c，
*/

#ifndef _ALG_DATA_H_
#define _ALG_DATA_H_

#include <stdint.h>

extern const int ALG_VERSION;
extern const int ALG_VERSION_BIG_PRIZE;

#define BET_LINE_TOTAL      			40               //压线数
//#define BET_LINE_TOTAL      			1               //压线数   test case
#define MAX_DIFF_NUM        			10               //难度个数
#define MAX_SEND_MAX_VALUE  			100000000        //最大炒场
#define ICON_NUM            			13               //图标种类
#define BOARD_ROW           			4                //盘面行数
#define BOARD_COLUMN        			5                //盘面列数
#define BOARD_ICON_NUM      			20               //盘面图标数量
#define BOARD_WILD_MAX      			12               //盘面百搭上限
#define ROLLER_ICON_LEN     			200              //每条滚轮图标最大数量
#define FREEGAME_THRESHOLD				3				 //進入FREEGAME條件
#define MUL_NUM     					3                //權重数量
#define MUL_WEIGHT_SUM     				100              //權重總和
#define BUY_FREEGAME_MUL_WEIGHT_SUM     1000             //購買免費遊戲權重總和
#define SCATTER_WEIGHT_SUM     			100              //Scatter權重總和
//#define SCATTER_WEIGHT_SUM1     		100              //Scatter權重總和  test case
//#define SCATTER_WEIGHT_SUM2     		100              //Scatter權重總和  test case
//#define SCATTER_WEIGHT_SUM3     		100              //Scatter權重總和  test case
//#define SCATTER_WEIGHT_SUM4     		100              //Scatter權重總和  test case
//#define SCATTER_WEIGHT_SUM5     		100              //Scatter權重總和  test case
#define SCATTER_WEIGHT_LEN     			1              	 //Scatter種類長度
#define FREEGAME_DEFAULT_NUM     		8              	 //免費遊戲預設次數

//#define DEBUG_PAYOUT_DIST				1
//#define FREEGAME_FLOW    				1
//#define PRINT_BOARD    					1
//#define SELF_ARRAY_TEST    					1

//#define REEL_DIST  						1

//#define FG_TEST    						1
#ifdef FG_TEST
	#define FG_NUM    					1
	#define FG_WILD_NUM					1
	#define FG_WILD_FINAL_NUM				1   
	#define FG_ADD_NUM					1
	#define FG_SC_DIST					1
	#define FG_SC_NUM						1
	//#define FG_ADD_ROUND    			1		//目前是壞的
#endif

//#define TEST_TRANVERSAL				1

//#define GET_RAND						1
//#define SRAND							1
#if defined(GET_RAND) || defined(SRAND)
	#define TEST_RAND_NUM		56
#endif

//#define WILD_MUL_DIST  					1  //記憶體影響到別人

extern const int E_ICON_NULL;
extern const int E_ICON_PIPC1;
extern const int E_ICON_PIPC2;
extern const int E_ICON_PIPC3;
extern const int E_ICON_PIPC4;
extern const int E_ICON_PIPC5;
extern const int E_ICON_PIPC6;
extern const int E_ICON_A;
extern const int E_ICON_K;
extern const int E_ICON_Q;
extern const int E_ICON_J;
extern const int E_ICON_10;
extern const int E_ICON_SC;
extern const int E_ICON_WILD;

extern const int E_PRIZETYPE_NORMAL;
extern const int E_PRIZETYPE_FREEGAME;

extern const uint8_t gLinePos[BET_LINE_TOTAL][BOARD_COLUMN];

extern const uint16_t gIconMulti[ICON_NUM+1][BOARD_COLUMN+1];

extern const uint16_t gRollerIconNum[BOARD_COLUMN];
extern const uint8_t gRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN];
extern const uint32_t gBaseGameWeight[MUL_NUM][2];

#define FREEGAME_TYPE			4                  //免费游戏种类
#define FREEGAME_ROUND_MAX		100         	   //免费游戏最大局数    //test case
#ifdef FG_NUM
	#define PRINT_MAX	100
#endif
	
extern const uint32_t gScatterInfoR[1][2];
extern const uint32_t gScatterInfoR5[1][2];
//extern const uint32_t gScatterInfoR1[1][2];//test case
//extern const uint32_t gScatterInfoR2[1][2];//test case
//extern const uint32_t gScatterInfoR3[1][2];//test case
//extern const uint32_t gScatterInfoR4[1][2];//test case
//extern const uint32_t gScatterInfoR5[1][2];//test case

extern const int gFreeGameInfo[FREEGAME_TYPE][2];
extern const uint16_t gFreeGameRollerIconNum[BOARD_COLUMN];
extern const uint8_t gFreeGameRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN];
extern const uint32_t gFreeGameWeight[MUL_NUM][2];

extern const uint16_t gBuyBaseGameRollerIconNum[BOARD_COLUMN];
extern const uint8_t gBuyBaseGameRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN];
extern const uint16_t gBuyFreeGameRollerIconNum[BOARD_COLUMN];
extern const uint8_t gBuyFreeGameRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN];
extern const uint32_t gBuyFreeGameWeight[MUL_NUM][2];

#endif