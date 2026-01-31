/*
除了个别要做数组下标的宏定义放data.h，其它配置统一放data.c，
*/

#ifndef _ALG_DATA_H_
#define _ALG_DATA_H_

#include <stdint.h>

extern const int ALG_VERSION;
extern const int ALG_VERSION_BIG_PRIZE;

#define BET_LINE_TOTAL      		5                //最多压线数
#define MAX_DIFF_NUM        		10               //难度个数
#define MAX_SEND_MAX_VALUE  		100000000        //最大炒场
#define ICON_NUM            		8                //图标种类
#define BOARD_ROW           		3                //盘面行数
#define BOARD_COLUMN        		3                //盘面列数
#define BOARD_ICON_NUM      		9                //盘面图标数量
#define ROLLER_ICON_LEN     		300              //每条滚轮图标最大数量
#define MUL_NUM     				6              	 //倍率数量
#define MUL_WEIGHT	    			10000         	 //MUL symbol weight 

//#define DEBUG_PAYOUT_DIST			1
//#define CAL_PRIZE_NUM    			1
//#define PRINT_BOARD    			1

extern const int E_ICON_NULL;
extern const int E_ICON_PIPC1;
extern const int E_ICON_PIPC2;
extern const int E_ICON_PIPC3;
extern const int E_ICON_PIPC4;
extern const int E_ICON_PIPC5;
extern const int E_ICON_PIPC6;
extern const int E_ICON_PIPC7;
extern const int E_ICON_WILD;

extern const int NORMAL_DIFF;
extern const double gTargetRTP[10][2];

extern const uint8_t gLinePos[BET_LINE_TOTAL][BOARD_COLUMN];

extern const uint16_t gIconMulti[ICON_NUM+1][BOARD_COLUMN+1];
extern const uint16_t gMulSymbolWeight[MUL_NUM][2];

extern const uint16_t gRollerIconNum[BOARD_COLUMN];
extern const uint8_t gRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN];


extern const uint16_t gBuyFreeGameRollerIconNum[BOARD_COLUMN];
extern const uint8_t gBuyFreeGameRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN];

#endif
